#include "bandit.h"
#include "utils/utils.h"
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>
#include <istream> 
#include <boost/lexical_cast.hpp>

Bandit::Bandit(uint _A, double _gamma,
				   double* _p): A(_A), gamma(_gamma), p(_p){
	//Look for max arm
	double maxp = 0;
	for(uint i=0;i<A;++i){
		if(p[i] > maxp){
			maxp = p[i];
			maxpi = i;
		}
	}

	//Read precomputed Gittins indices from file
	maxG = 40; //Max alpha/beta value precomputed

	
	std::string filename("gittins_beta_");
	filename.append(boost::lexical_cast<std::string>(rint(gamma*100)));
	filename.append("_");
	filename.append(boost::lexical_cast<std::string>(maxG));
	filename.append("_");
	filename.append(boost::lexical_cast<std::string>(maxG));
	if (!boost::filesystem::exists(filename.c_str()))
		assert(0);

	boost::iostreams::stream<boost::iostreams::file_source>
		file(filename.c_str());

	gittinsi = new double*[maxG];
	for(uint i=0;i<maxG;++i)
		gittinsi[i] = new double[maxG];
	std::string line;

	//FIRST FORMAT:  all indices for beta=1, all indices for beta = 2 ....
	//uint i=0;
	//while (std::getline(file, line)) {
	//	gittinsi[i%maxG][i/maxG] = utils::StringToFloat(line);	
	//	i++;
	//}
	
	//SECOND FORMAT (pdf table -> text) all indices for beta 1 - alpha 1 to 10, all indices for beta 2 - alpha 1 to 10, ... all indices for beta 1 - alpha 11 to 20, ...
	uint beta = 1;
	uint alpha = 1;
	uint page=0;
	while (std::getline(file, line)) {
		assert(alpha <= maxG && beta <= maxG);
		gittinsi[alpha-1][beta-1] = utils::StringToFloat(line);	
		alpha++;
		if(alpha%10 == 1 && alpha != page*10+1){
			beta++;
			alpha = page*10+1;
		}
		if(beta == maxG+1){
			page++;
			beta=1;
			alpha = page*10+1;
		}
	}
	
}
Bandit::~Bandit(){
	for(uint i=0;i<maxG;++i)
		delete[] gittinsi[i];
	delete[] gittinsi;
}

//Execute selected arm and return reward
uint Bandit::step(uint arm){
	if(utils::rng.rand_closed01() < p[arm])
		return 1;
	else
		return 0;
}

uint Bandit::gittinsLearn(std::vector<uint>& Rhist, std::vector<uint>& optArm, uint N){


	//Fitted A,B,C values (See Gittins 1989, Table 12,13,14 and Equation 7.16)
	double meanLambda [13] = {0.025, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95, 0.975};
	//Values for gamma=0.99 - precise up to n=150
	double fittedA [13]  = {6.1145, 10.3081, 13.4238, 17.1924, 20.2994, 21.5139, 22.9575, 24.9623, 
		27.2915, 31.0320, 38.4591, 49.5095, 65.9106};
	double fittedB [13] = {1.7222, 1.6483, 1.6080, 1.5820, 1.5709, 1.5716, 1.5795, 1.5745, 1.5796,
		1.5892, 1.62, 1.6793, 1.7798};
	double fittedC [13] = {36.7720, -119.1096, -122.1469, -139.2893, -168.5174, -152.4617,
		-155.0706, -150.1690, -148.8783, -151.9041, -126.5893, -79.0757, 36.7720};

	
	uint** counts = new uint*[A];
	for(uint a=0;a<A;++a){
		counts[a] = new uint[2]; //0-->alpha (success) 1-->beta (failure)
		//IMPORTANT NOTE: effective initial counts are 1 since precomputed
		//gittins indices start at (1,1)
		counts[a][0] = 0; //Init to uniform prior 
	 	counts[a][1] = 0;
	}
	uint step = 0;
	double discount=1;
	double sumD=0;double sumC = 0;
	double GittinsIndex;
	bool outOfIndices = false;
	while(!outOfIndices && step < N){
		//Get arm with max index (with random tie breaking)
		std::vector<uint> maxga;
		double maxg = 0;
		for(uint a=0;a<A;++a){
			//std::cout << "Arm " << a << " (" 
			//	<< counts[a][0] << "," << counts[a][1] << ")" <<  gittinsi[counts[a][0]][counts[a][1]] << std::endl;
			if(counts[a][0] < maxG-20 && counts[a][1] < maxG-20)
				GittinsIndex = gittinsi[counts[a][0]][counts[a][1]];
			else{
				//Compute from fitted values
				//First find matching mean
				double alpha = counts[a][0] + 1;
				double beta = counts[a][1] + 1;
				double n = alpha+beta;
				double lambda = alpha/n;
				uint i;
				for(i=0;i<13;++i){
					if(meanLambda[i] > lambda)
						break;
				}
				//Interpolate A-B-C values linearly
				double z = (lambda-meanLambda[i-1])/(meanLambda[i]-meanLambda[i-1]);
				double Ai = fittedA[i-1]*(1-z)+z*fittedA[i];
				double Bi = fittedB[i-1]*(1-z)+z*fittedB[i];
				double Ci = fittedC[i-1]*(1-z)+z*fittedC[i];
				//std::cout << lambda << " " << z << " " << Ai << " " << Bi << " " << Ci << std::endl;
				GittinsIndex = alpha/n + 1.0/((Ai+Bi*n+Ci/n)*sqrt(1-gamma)*n/(sqrt(alpha)*sqrt(beta))); 
				//std::cout << alpha << " " << beta << " " << GittinsIndex << std::endl;
			}
			if(GittinsIndex > maxg){
				maxga.clear();
				maxga.push_back(a);
				maxg = GittinsIndex;
			}
			if(fabs(GittinsIndex-maxg)<0.0000001){
				maxga.push_back(a);
			}
		}
		//Play max arm
		uint maxga_sample;
		if(maxga.size() > 1)
			maxga_sample = maxga[rand() % maxga.size()];
		else
			maxga_sample = maxga[0];
		
		uint result = this->step(maxga_sample);

		//std::cout << "GI " << step << " " << maxga_sample << " " << result << std::endl;
		sumD += result*discount;
		sumC += result;
		discount *= gamma;

		//Update counts and check outOfIndices cond
		counts[maxga_sample][(result+1)%2]++;
		if(counts[maxga_sample][(result+1)%2] > maxG-1){
			if(fabs(gamma-0.99)>0.00000001){
				outOfIndices = true;
			}
		}
		
		step++;

		//Record results
		Rhist.push_back(result);
		if(maxga_sample == this->maxpi)
			optArm.push_back(1);
		else
			optArm.push_back(0);
		
	}
	std::cout << "GI CR " << sumC << "   DR " << sumD << std::endl;
	return step;
}

//Choose bandits greedily according to max posterior mean (+ random tie breaking)
void Bandit::postMean(std::vector<uint>& Rhist, std::vector<uint>& optArm,
size_t N){
	//Initialize transition counts for posterior estimation
	double** counts = new double*[A];
	for(uint a=0;a<A;++a){
		counts[a] = new double[2]; //0-->alpha (success) 1-->beta (failure)
		counts[a][0] = 1;  
		counts[a][1] = 1;
	}
	double discount = 1;
	double sumD = 0; double sumC = 0;

	std::vector<uint> bestArms;

	for(size_t t = 0; t < N; ++t){
		bestArms.clear();
		
		double maxMean = 0;
		double meana;
		//kind of wasteful but we don't care about performance here
		for(uint k=0; k < A;++k){
			meana = counts[k][0]/(counts[k][0]+counts[k][1]);
			if(fabs(meana-maxMean)<0.0000001){
				bestArms.push_back(k);
			}
			if(meana > maxMean){
				maxMean = meana;
				bestArms.clear();
				bestArms.push_back(k);
			}
		}
		//Tie-breaking
		uint bestArm;
		if(bestArms.size() > 1)	
			bestArm = bestArms[rand() % bestArms.size()];
		else
			bestArm = bestArms[0];

		//Execute believed best arm 
		uint r = step(bestArm);

		//std::cout << "PM " << t << " " << bestArm << " " << r << std::endl;
		sumD += r*discount;
		sumC += r;
		discount *= gamma;

	
		//Execute current policy and record reward
		Rhist.push_back(r);
		if(bestArm == maxpi)
			optArm.push_back(1);
		else
			optArm.push_back(0);

		//Update counts
		counts[bestArm][(r+1)%2] += 1;
	
	}
	std::cout << "PM CR " << sumC << "   DR " << sumD << std::endl;
	for(uint a=0;a<A;++a){
		delete[] counts[a];
	}
	delete[] counts;

}

void Bandit::sboss(std::vector<uint>& Rhist, std::vector<uint>& optArm,
		           size_t K, double delta, size_t N){


	//Initialize transition counts for posterior estimation
	double** counts = new double*[A];
	for(uint a=0;a<A;++a){
		counts[a] = new double[2]; //0-->alpha (success) 1-->beta (failure)
		counts[a][0] = 1;  
	 	counts[a][1] = 1;
	}

	//posterior at last model resampling
	double aLast = 0;
	double bLast = 0;
	
	uint bestArm = 0;
	bool do_sample = true;
	size_t timeSinceLastSample = 0;

	double* ptemp = new double[2];

	for(size_t t = 0; t < N; ++t){
		if(do_sample){
			double bestArmP = 0;	
			//Sample K models from posterior
			//and select best arm across all models
			for(uint k=0;k<K;++k){
				for(uint a=0;a<A;++a){
					utils::sampleDirichlet(ptemp,counts[a],2);
					if(ptemp[0] > bestArmP){
						bestArm = a;
						bestArmP = ptemp[0];
					}
				}
			}

			do_sample = false;
			
			timeSinceLastSample = 0;
			aLast = counts[bestArm][0];
			bLast = counts[bestArm][1];
		}
		else{
			timeSinceLastSample++;
		}

		//Execute believed best arm 
		uint r = step(bestArm);

		//Execute current policy and record reward
		Rhist.push_back(r);
		if(bestArm == maxpi)
			optArm.push_back(1);
		else
			optArm.push_back(0);

		//Update counts
		counts[bestArm][(r+1)%2] += 1;

		//Resampling criterion
		if(posteriorDeviation(counts[bestArm],
					aLast,bLast) > delta){
			do_sample = true;
		}
		
	}
	for(uint a=0;a<A;++a){
		//std::cout << "Arm " << a << " (" 
		//	<< counts[a][0] << "," << counts[a][1] << ")" << std::endl;

		delete[] counts[a];
	}
	delete[] counts;
	delete ptemp;
}

//Compute the deviation of the means from one posterior to the other,
//as done in Precup, Castro 2010.
//(for BOSS)
double Bandit::posteriorDeviation(double* counts1,
		                      double acounts2, double bcounts2){
	double sum1= counts1[0]+counts1[1];
	double dev = 0;
	double tmp;
	tmp = fabs((counts1[0])/(counts1[0]+counts1[1])-(acounts2/(acounts2+bcounts2)));
	//Divide by variance of the marginal for s
	dev += tmp/sqrt(counts1[0]*counts1[1]);
	tmp = fabs((counts1[1])/(sum1)-(bcounts2/(acounts2+bcounts2)));
	dev += tmp/sqrt(counts1[0]*counts1[1]);
	dev *= sqrt((sum1*sum1*(sum1+1)));
	return dev;
}

