#include "LazyInfGridSampler.h"

#if defined(GX)
	#include <GL/glfw.h>
#endif

#include "envs/flat/lazyInfGrid.h"

LazyInfGridSampler::LazyInfGridSampler(double _gamma,
		int** _ATobserved,
		uint** _counts_col,
		uint** _counts_row,
		std::vector<Pair> *_PairObserved,
		uint _boundX, uint _boundY,
		uint STARTX, uint STARTY,
		double* _beta_params_col,
		double* _beta_params_row,
		double* _pcol,
		double* _prow):
gamma(_gamma),ATobserved(_ATobserved), counts_col(_counts_col),
counts_row(_counts_row),PairObserved(_PairObserved),boundX(_boundX),boundY(_boundY),
beta_params_col(_beta_params_col),
beta_params_row(_beta_params_row),
pcol(_pcol), prow(_prow){

	simulator = new LazyInfGrid(gamma,this);
	simulator->setSTART(STARTX,STARTY);

	// -1  -> Not sampled  (Or observed in ATobserved)
	// 0-1 -> sampled absent/present reward  
	ATmap = new int*[boundX];
	for(uint xx=0;xx<boundX;++xx){
		ATmap[xx] = new int[boundY];
		std::fill(ATmap[xx],ATmap[xx]+boundY,-1);	
	}
	
	param = new double[2];
	beta_params = new double[2];

	pcol_un = new double[boundX];
	pcol_age = new uint[boundX];

	prow_un = new double[boundY];
	prow_age = new uint[boundY];

	std::fill(pcol_un,pcol_un+boundX,-1);
	std::fill(pcol_age,pcol_age+boundX,0);
	
	std::fill(prow_un,prow_un+boundY,-1);
	std::fill(prow_age,prow_age+boundY,0);

	colmeanf = beta_params_col[1]/(beta_params_col[0]+beta_params_col[1]);
	rowmeanf = beta_params_row[1]/(beta_params_row[0]+beta_params_row[1]);

	accepts = 0;
	updates = 0;
	updatesAT = 0;
	
	lastx = 0;
	lasty = 0;

	for(uint b=0;b<10;++b){	
		updateP();
	}	
	
#if defined(GX)
	SRObserved.reserve(300);
#endif
}

void LazyInfGridSampler::setLastPos(uint x, uint y){
		lastx = x, lasty = y;
		
}

//This is purely for rendering purposes
void LazyInfGridSampler::setLastPMeans(double* _pcol_mean,double* _prow_mean,
		uint* _pcol_upcounts, uint* _prow_upcounts, uint lasta){
#if defined(GX)
	pcol_mean = _pcol_mean;
	pcol_upcounts = _pcol_upcounts;
	prow_mean = _prow_mean;
	prow_upcounts = _prow_upcounts;
	
	uint smooth_count = 500;
	//Shift array based on last action and reset counts for smooth transitions
	//FIXME ugly code before deadline
	if(lasta == 2){ //Shift prow up
		for(uint i=21;i>=1;--i){
			prow_mean[i-1] = prow_mean[i-1]/prow_upcounts[i-1];
			prow_mean[i] = smooth_count*prow_mean[i-1];
			prow_upcounts[i] = smooth_count;

			pcol_mean[i] = smooth_count*pcol_mean[i]/pcol_upcounts[i];
			pcol_upcounts[i] = smooth_count;
		}
		prow_mean[0] = smooth_count*(1-rowmeanf);
		prow_upcounts[0] = smooth_count;
		
		pcol_mean[0] = smooth_count*pcol_mean[0]/pcol_upcounts[0];
		pcol_upcounts[0] = smooth_count;
	}
	if(lasta == 3){ //Shift col up
		for(uint i=21;i>=1;--i){
			pcol_mean[i-1] = pcol_mean[i-1]/pcol_upcounts[i-1];
			pcol_mean[i] = smooth_count*pcol_mean[i-1];
			pcol_upcounts[i] = smooth_count;
			
			prow_mean[i] = smooth_count*prow_mean[i]/prow_upcounts[i];
			prow_upcounts[i] = smooth_count;
		}
		pcol_mean[0] = smooth_count*(1-colmeanf);
		pcol_upcounts[0] = smooth_count;
		prow_mean[0] = smooth_count*prow_mean[0]/prow_upcounts[0];
		prow_upcounts[0] = smooth_count;
	}
	if(lasta == 0){ //Shift prow down
		for(uint i=0;i<=20;++i){
			prow_mean[i+1] = prow_mean[i+1]/prow_upcounts[i+1];
			prow_mean[i] = smooth_count*prow_mean[i+1];
			prow_upcounts[i] = smooth_count;

			pcol_mean[i] = smooth_count*pcol_mean[i]/pcol_upcounts[i];
			pcol_upcounts[i] = smooth_count;
		}
		prow_mean[21] = smooth_count*(1-rowmeanf);
		prow_upcounts[21] = smooth_count;
		
		pcol_mean[21] = smooth_count*pcol_mean[21]/pcol_upcounts[21];
		pcol_upcounts[21] = smooth_count;
	}
	if(lasta == 1){ //Shift col down
		for(uint i=0;i<=20;++i){
			pcol_mean[i+1] = pcol_mean[i+1]/pcol_upcounts[i+1];
			pcol_mean[i] = smooth_count*pcol_mean[i+1];
			pcol_upcounts[i] = smooth_count;
			
			prow_mean[i] = smooth_count*prow_mean[i]/prow_upcounts[i];
			prow_upcounts[i] = smooth_count;
		}
		pcol_mean[21] = smooth_count*(1-colmeanf);
		pcol_upcounts[21] = smooth_count;
		prow_mean[21] = smooth_count*prow_mean[21]/prow_upcounts[21];
		prow_upcounts[21] = smooth_count;
	}

	if(prow_mean[11] < 0.00000001)
		prow_mean[11] = smooth_count*(1-rowmeanf);
	if(pcol_mean[11] < 0.00000001)
		pcol_mean[11] = smooth_count*(1-colmeanf);
/*
	for(uint i=0;i<22;++i)
		std::cout << prow_mean[i] << ":" << prow_upcounts[i] << " ";
	std::cout << std::endl; */
#endif
}

LazyInfGridSampler::~LazyInfGridSampler(){
	if(simulator)
		delete simulator;

	for(uint xx=0;xx<boundX;++xx)
		delete[] ATmap[xx];
	delete[] ATmap;

//	delete[] pcol_prop;
//	delete[] pcol_dirty;
	delete[] pcol_un;
	delete[] pcol_age;

//	delete[] prow_prop;
//	delete[] prow_dirty;
	delete[] prow_un;
	delete[] prow_age;

	delete[] param;
	delete[] beta_params;
	

//	double accratio = ((double)accepts)/updates;
//	if(accratio < 0.2){	
//		std::cout << "Accept. Ratio: " << accratio << std::endl;
//	}
}

void LazyInfGridSampler::updateP(){
	//Proposal P params
	uint xx,yy;
	uint kk;

	//for(uint kk=0; kk<PairObserved->size();++kk){
	kk = rand() % PairObserved->size();
	xx = PairObserved->at(kk).first;
	yy = PairObserved->at(kk).second;

	uint xx2,yy2;

	//COLUMN XX PROPOSAL ----------------------------
	double cn1 = counts_col[xx][0];
	double cn2 = 0.5*counts_col[xx][1]*(1-rowmeanf); 
/*
	for(kk=0; kk<PairObserved->size();++kk){
		xx2 = PairObserved->at(kk).first;
		yy2 = PairObserved->at(kk).second;
		if(xx2 == xx && prow[yy2] > 0){
			cn2 += 0.5*(1-ATobserved[xx2][yy2])*std::max(0.05,std::min(0.95,prow[yy2])); 	
			//std::cout << yy2 << ":" << prow[yy2] << " ";
		}
	}*/
	//Avoid peaked proposals, since proposal is biased	
	double cn12sum = cn1+cn2;
	if(cn12sum > 5){
		cn1 = 5*cn1/(cn12sum);
		cn2 = 5*cn2/(cn12sum);
	}
	beta_params[0] = beta_params_col[0] + cn1;
	beta_params[1] = beta_params_col[1] + cn2;

	utils::sampleDirichlet(param,beta_params,2);
	pcol_propxx = param[0];

	if(pcol[xx] < 0){ //First time we sample this parameter (need to start the chain)
		utils::sampleDirichlet(param,beta_params,2);
		pcol[xx] = param[0];
	}

	//ROW YY PROPOSAL ----------------------------
				
	double rn1 = counts_row[yy][0];
	double rn2 = 0.5*counts_row[yy][1]*(1-colmeanf);
/*
	for(kk=0; kk<PairObserved->size();++kk){
		xx2 = PairObserved->at(kk).first;
		yy2 = PairObserved->at(kk).second;
		if(yy2 == yy && pcol[xx2] > 0){
			rn2 += 0.5*(1-ATobserved[xx2][yy2])*std::max(0.05,std::min(0.95,pcol[xx2])); 	
		}
	}*/
	double rn12sum = rn1+rn2;
	if(rn12sum > 5){
		rn1 = 5*rn1/(rn12sum);
		rn2 = 5*rn2/(rn12sum);
	}
	beta_params[0] = beta_params_row[0] + rn1; 
	beta_params[1] = beta_params_row[1] + rn2;

	utils::sampleDirichlet(param,beta_params,2);
	prow_propyy = param[0];

	//	std::cout << "ROW: " << yy << " " << param[0] << " " << beta_params[0] << " " << beta_params[1] << std::endl;
	if(prow[yy] < 0){ //First time we sample this parameter (need to start the chain)
		utils::sampleDirichlet(param,beta_params,2);
		prow[yy] = param[0];
	}

	//Compute Acceptance probability
	// min(1,  exp( log( P( D | p') P(p') / P(D | p) P(p)))) 
	double logSum = 0;
	for(kk=0; kk<PairObserved->size();++kk){
		
		xx2 = PairObserved->at(kk).first;
		yy2 = PairObserved->at(kk).second;
		
		if(pcol[xx2] > 0 && prow[yy2] > 0){

			if(xx2 == xx || yy2 == yy){		
				if(ATobserved[xx2][yy2]){ // observed 1 ?
					//logSum += log(pcol_prop[xx]*prow_prop[yy]);
					if(xx2 == xx && yy2 != yy)
						logSum += log(pcol_propxx*prow[yy2]);
					else if(xx2 != xx && yy2 == yy)
						logSum += log(pcol[xx2]*prow_propyy);
					else
						logSum += log(pcol_propxx*prow_propyy);
					logSum -= log(pcol[xx2]*prow[yy2]);
					//std::cout << xx2 << ":" << yy2 << ";" << xx << ":" << yy << " " << pcol[xx2] << " " << prow[yy2] << " " << pcol[xx] << " " << prow[yy] << std::endl;
				}
				else{ // observed 0 ?
					//logSum += log(1-pcol_prop[xx]*prow_prop[yy]);
					if(xx2 == xx && yy2 != yy)
						logSum += log(1-pcol_propxx*prow[yy2]);
					else if(xx2 != xx && yy2 == yy)
						logSum += log(1-pcol[xx2]*prow_propyy);
					else
						logSum += log(1-pcol_propxx*prow_propyy);
					logSum -= log(1-pcol[xx2]*prow[yy2]);
					//std::cout << pcol[xx2] << " " << prow[yy2] << " " << pcol[xx] << " " << prow[yy] << std::endl;
				}
			}
		}
	}
	//	std::cout << pcol[xx] << " " << pcol_propxx << std::endl;

	logSum -= cn1*log(pcol_propxx);
	logSum -= cn2*log(1-pcol_propxx);
	logSum += cn1*log(pcol[xx]);
	logSum += cn2*log(1-pcol[xx]);

	logSum -= rn1*log(prow_propyy);
	logSum -= rn2*log(1-prow_propyy);
	logSum += rn1*log(prow[yy]);
	logSum += rn2*log(1-prow[yy]);
	
	double acceptProb = exp(logSum);
	
	if(acceptProb > 1  || utils::rng.rand_closed01() < acceptProb){
			//ACCEPT
			pcol[xx] = pcol_propxx;
			prow[yy] = prow_propyy;
			//std::cout << "ACCEPT" << std::endl;
			accepts++;
	}
	updates++;

	#if defined(GX)
	
		for(uint i=0;i<22;++i){
			xx = i + lastx - 11;
			if(lastx > 0){
			if(pcol[xx] > 0){
				pcol_mean[i] += pcol[xx];
				pcol_upcounts[i]++;
			}}
			yy = i + lasty - 11;
			if(lasty > 0){
			if(prow[yy] > 0){
				prow_mean[i] += prow[yy];
				prow_upcounts[i]++;
			}}
		}
	
	#endif

}

//Sample state attribute (AT) only when necessary
uint LazyInfGridSampler::getAT(uint xx, uint yy){
	if(ATmap[xx][yy] < updatesAT){ // Reward at xx-yy not yet set during this simulation?
		if(ATobserved[xx][yy] == -1){  //Have we observed reward directly?
			//Draw reward 
			//First get pcol_un[xx] and prow_un[yy]
			if(pcol[xx] < 0){  //Not MCMCing that variable yet
				if(pcol_age[xx] < updatesAT){ //Change prior sample if old (dates from previous simulation)
					//Sample from prior
					utils::sampleDirichlet(param,beta_params_col,2);
					pcol_un[xx] = param[0];
					pcol_age[xx] = updatesAT;
				}
			}else{
				pcol_un[xx] = pcol[xx];
			}
			if(prow[yy] < 0){ 
				if(prow_age[yy] < updatesAT){
					//Sample from prior
					utils::sampleDirichlet(param,beta_params_row,2);
					prow_un[yy] = param[0];
					prow_age[yy] = updatesAT;
				}
			}else{
				prow_un[yy] = prow[yy];
			}
			ATmap[xx][yy] = updatesAT; 
			if(utils::rng.rand_closed01() < pcol_un[xx]*prow_un[yy]){
				#if defined(GX)
					SRObserved.push_back(Pair(xx,yy));
				#endif
				return 1;
			}
			else
				return 0;
		}
		else
			return 0; //If reward was observed, it is no longer there (observation => consumption)
	}
	else
		return 0;
	
}

SIMULATOR2D* LazyInfGridSampler::updateMDPSample_ind(){
	//FIXME thinning should be determined empirically
	for(uint b=0;b<10;++b){	
		updateP();
	}
	assert(0);
	return updateMDPSample();
}
SIMULATOR2D* LazyInfGridSampler::updateMDPSample(){
	assert(updatesAT < 10000000);  //Reset counter routine todo

	//Reset unobserved AT
	updatesAT++;	
	
	for(uint b=0;b<20;++b){	
		updateP();
	}	

	simulator->reset();
	#if defined(GX)
		SRObserved.clear();
	#endif
	return (SIMULATOR2D*) simulator;	
}


void LazyInfGridSampler::render(double startx, double starty, double sqsz){
#if defined(GX)

	GLUquadricObj* quadric = gluNewQuadric();
	gluQuadricDrawStyle(quadric, GLU_FILL);
	double tmp;
	for(uint i=1;i<21;++i){
		if(pcol_mean[i] > 0)
			tmp = sqrt(pcol_mean[i]/pcol_upcounts[i]);
		else
			tmp = sqrt(1-colmeanf);
		glColor3f(0.1725,0.1686,0.5294);
		glPushMatrix();
		glTranslated(40.0+(i-1)*sqsz*2,30.0,0.0);
		gluDisk(quadric,0,10*tmp,32,1);
		glPopMatrix();	
	}
	for(uint i=1;i<21;++i){
		if(prow_mean[i] > 0)
			tmp = sqrt(prow_mean[i]/prow_upcounts[i]);
		else
			tmp = sqrt(1-rowmeanf);
		glColor3f(0.8745,0.4745,0.0745);
		glPushMatrix();
		glTranslated(15.0,60.0+(i-1)*sqsz*2,0.0);
		gluDisk(quadric,0,10*tmp,32,1);
		glPopMatrix();	
	}
	gluDeleteQuadric(quadric);

		
	uint S=40;
	sqsz = sqsz/2;

	for(uint xx=0; xx < S; xx++)
	{
		for(uint yy=0; yy < S; yy++)
		{
			int ii = ATobserved[xx+lastx-20][yy+lasty-20];
			int jj = ATmap[xx+lastx-20][yy+lasty-20];
			if(ii>=0){
				glColor3f(simulator->ATcolor[ii][0],simulator->ATcolor[ii][1],simulator->ATcolor[ii][2]+utils::rng.rand_closed01()/15);
				glPushMatrix();
				glTranslated(startx+xx*sqsz,starty+yy*sqsz,0.0);
				glBegin(GL_QUADS);
				glVertex3f(0.0f, sqsz, 0.0f);
				glVertex3f(sqsz,sqsz, 0.0f);
				glVertex3f(sqsz,0.0f, 0.0f);
				glVertex3f(0.0f,0.0f, 0.0f);
				glEnd();
				glPopMatrix();
			}
			else if(jj == updatesAT){
				jj = 0;
				glColor3f(simulator->ATcolor[jj][0],simulator->ATcolor[jj][1],simulator->ATcolor[jj][2]+utils::rng.rand_closed01()/15);
				glPushMatrix();
				glTranslated(startx+xx*sqsz,starty+yy*sqsz,0.0);
				glBegin(GL_QUADS);
				glVertex3f(0.0f, sqsz, 0.0f);
				glVertex3f(sqsz,sqsz, 0.0f);
				glVertex3f(sqsz,0.0f, 0.0f);
				glVertex3f(0.0f,0.0f, 0.0f);
				glEnd();
				glPopMatrix();
			}
		}
	}
	double xx,yy;
	uint jj;	
	for(uint ii=0;ii<SRObserved.size();++ii){
		jj=1;
		xx=(double)SRObserved[ii].first-lastx+20;
		yy=(double)SRObserved[ii].second-lasty+20;
		if(xx >= 0 && xx < S && yy >= 0 && yy < S){
			
			glColor3f(simulator->ATcolor[jj][0],simulator->ATcolor[jj][1],simulator->ATcolor[jj][2]);
			glPushMatrix();
			glTranslated(startx+xx*sqsz,starty+yy*sqsz,0.0);
			glBegin(GL_QUADS);
			glVertex3f(0.0f, sqsz, 0.0f);
			glVertex3f(sqsz,sqsz, 0.0f);
			glVertex3f(sqsz,0.0f, 0.0f);
			glVertex3f(0.0f,0.0f, 0.0f);
			glEnd();
			glPopMatrix();
		}

	}
#endif
}
