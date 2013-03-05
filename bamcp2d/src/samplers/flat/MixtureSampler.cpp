#include "MixtureSampler.h"

MixtureSampler::MixtureSampler(double _gamma,uint _NAT, int** _ATobserved,uint** _counts,
		uint STARTX, uint STARTY, uint STARTXP, uint STARTYP):
gamma(_gamma),NAT(_NAT),ATobserved(_ATobserved), counts(_counts) {

	simulator = new SqGrid(XG,YG,NAT,gamma);
	
	simulator->setPREDVARS(PREDCOSTCONST,PREDWAITCONST);
	simulator->setSTART(STARTX,STARTY);
	simulator->setSTARTP(STARTXP,STARTYP);

	richDirParameters = new double[NAT];
	poorDirParameters = new double[NAT];

	//FIXME Manually modify in LazyMixture as well...
	if(NAT == 3){
		richDirParameters[0] = 1.2;
		richDirParameters[1] = 0.6;
		richDirParameters[2] = 0.4;
		poorDirParameters[0] = 1.4;
		poorDirParameters[1] = 0.4;
		poorDirParameters[2] = 0.6;

	}else{
		richDirParameters[0] = 1.8;
		richDirParameters[1] = 0.6;
		richDirParameters[2] = 0.4;
		richDirParameters[3] = 0.04;

		poorDirParameters[0] = 2.0;
		poorDirParameters[1] = 0.4;
		poorDirParameters[2] = 0.6;
		poorDirParameters[3] = 0.08;

		simulator->setATr(1,0.5);
		simulator->setATr(3,1.5);
	}

	multParams = new double*[NQ];
	for(uint qq=0;qq<NQ;++qq){
		multParams[qq] = new double[NAT];
		std::fill(multParams[qq],multParams[qq]+NAT,1.0/NAT); //start values
	}

	countsTmp = new double[NAT];

	//burn in the chain
	for(uint b=0;b<50;++b){	
		updateR();
		updateMultParams(); 
	}

}
MixtureSampler::~MixtureSampler(){
	if(simulator)
		delete simulator;

	delete[] richDirParameters;
	delete[] poorDirParameters;
	for(uint qq=0;qq<NQ;++qq)
		delete[] multParams[qq];
	delete[] multParams;
	delete[] countsTmp;
}


//FIXME potential for caching...
void MixtureSampler::updateR(){
	double normalizer = 0;
	for(uint rr=0;rr<NQ;++rr){
		double sumLog = 0;
		for(uint qq=0;qq<NQ;++qq){
			if(qq!=rr){
				for(uint at=0;at<NAT;++at)
					sumLog += (poorDirParameters[at]-1)*log(multParams[qq][at]);
			}
			else{
				for(uint at=0;at<NAT;++at)
					sumLog += (richDirParameters[at]-1)*log(multParams[qq][at]);
			}
		}
		Rp[rr] = exp(sumLog);
		normalizer += exp(sumLog);	
	}
	for(uint qq=0;qq<NQ;++qq)
		Rp[qq] = Rp[qq] / normalizer;

	utils::rng.multinom(Rp, NQ, R);

}

void MixtureSampler::updateMultParams(){
	for(uint qq=0;qq<NQ;++qq){
		if(qq == R)
			memcpy(countsTmp,richDirParameters,NAT*sizeof(double));
		else
			memcpy(countsTmp,poorDirParameters,NAT*sizeof(double));
		for(uint at=0;at<NAT;++at)
			countsTmp[at] += counts[qq][at];

		utils::sampleDirichlet(multParams[qq], countsTmp,NAT);
	}
}
SIMULATOR2D* MixtureSampler::updateMDPSample_ind(){
	//Get near-independent samples through thinning
	for(uint b=0;b<20;++b){	
		updateR();
		updateMultParams(); 
	}
	return updateMDPSample();
}

SIMULATOR2D* MixtureSampler::updateMDPSample(){

	//Do one pass of Gibb sampling
	updateR();
	updateMultParams();

	
	//Draw unobserved attributes for each position in quadrant from multinomial params
	assert(NQ==4);
	uint samp;
	for(uint qq=0;qq<4;++qq){
		uint offsetx = (qq/2)*(XG/2);
		uint offsety = (qq%2)*(YG/2);
		for(uint xx=offsetx;xx<XG/2+offsetx;xx++){
			for(uint yy=offsety;yy<YG/2+offsety;yy++){
				if(ATobserved[xx+1][yy+1] == -1){ //if unobserved (note: change of coordinates from agent to world coords
					utils::rng.multinom(multParams[qq],NAT,samp);		
					simulator->setATxy(samp,xx,yy);
				}
				else
					simulator->setATxy(ATobserved[xx+1][yy+1],xx,yy);
			}
		}
	}


/*
	//Generative model
	assert(XG % 4 == 0 && YG % 4 == 0);
	//Quadrants:
	//
	//  1 3
	//  0 2

	//First select which quadrant is "rich"
	// R ~ Discrete(1/4,1/4,1/4,1/4)
	uint rich = 0;
	double draw = utils::rng.rand_closed01();
	if(draw < 0.25)
		rich = 0;
	else if(draw < 0.5)
		rich = 1;
	else if(draw < 0.75)
		rich = 2;
	else 
		rich = 3;
	//------------------------------------
	//Then draw multinomial params from dirichlet
	//for each quadrant
	double* probs = new double[3];
	//uint size = XG*YG/4; //Size of quadrant
	//double* samp = new double[size];
	uint samp = 0;
	for(uint ii=0;ii<4;++ii){
		if(rich != ii)
			utils::sampleDirichlet(probs, poorDirParameters,3);
		else
			utils::sampleDirichlet(probs, richDirParameters,3);

		//Draw attributes for each position in quadrant from multinomial params
		//utils::rng.multinom(size,probs,3,samp);
		uint jj=0;
		uint offsetx = (ii/2)*(XG/2);
		uint offsety = (ii%2)*(YG/2);
		for(uint xx=offsetx;xx<XG/2+offsetx;xx++){
			for(uint yy=offsety;yy<YG/2+offsety;yy++){	
				utils::rng.multinom(probs,3,samp);		
				simulator->setATxy(samp,xx,yy);
				jj++;
			}
		}
	}
	
	//delete[] samp;
	delete[] probs;
	*/

	return (SIMULATOR2D*) simulator;	
}


