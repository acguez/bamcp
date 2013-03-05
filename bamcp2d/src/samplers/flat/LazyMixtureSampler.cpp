#include "LazyMixtureSampler.h"

#if defined(GX)
	#include <GL/glfw.h>
#endif

LazyMixtureSampler::LazyMixtureSampler(double _gamma,uint _NAT,
		int** _ATobserved,uint** _counts,
		uint STARTX, uint STARTY, uint STARTXP, uint STARTYP):
gamma(_gamma),NAT(_NAT),ATobserved(_ATobserved), counts(_counts) {

	simulator = new LazySqGrid(XG,YG,NAT,gamma,this);
	
	simulator->setPREDVARS(PREDCOSTCONST,PREDWAITCONST);
	simulator->setSTART(STARTX,STARTY);
	simulator->setSTARTP(STARTXP,STARTYP);

	richDirParameters = new double[NAT];
	poorDirParameters = new double[NAT];
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
	for(qq=0;qq<NQ;++qq){
		multParams[qq] = new double[NAT];
		std::fill(multParams[qq],multParams[qq]+NAT,1.0/NAT); //start values
	}
	
	ATtmp = new int*[XG+2];
	for(uint xx=0;xx<XG+2;++xx){
		ATtmp[xx] = new int[YG+2];
		std::fill(ATtmp[xx],ATtmp[xx]+YG+2,-1);	
	}

	countsTmp = new double[NAT];
	
	XGhalf = (XG+2)/2;
	YGhalf = (YG+2)/2;

	//burn in the chain
	for(uint b=0;b<50;++b){	
		updateR();
		updateMultParams(); 
	}
	
}
LazyMixtureSampler::~LazyMixtureSampler(){
	if(simulator)
		delete simulator;

	delete[] richDirParameters;
	delete[] poorDirParameters;
	for(qq=0;qq<NQ;++qq)
		delete[] multParams[qq];
	for(uint xx=0;xx<XG;++xx)
		delete[] ATtmp[xx];
	delete[] ATtmp;
	delete[] multParams;
	delete[] countsTmp;
}


//FIXME potential for caching...
void LazyMixtureSampler::updateR(){
	double normalizer = 0;
	for(uint rr=0;rr<NQ;++rr){
		double sumLog = 0;
		for(qq=0;qq<NQ;++qq){
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
	for(qq=0;qq<NQ;++qq)
		Rp[qq] = Rp[qq] / normalizer;

	//std::cout << Rp[0] << " " << Rp[1] << " " << Rp[2] << " " << Rp[3] << std::endl;
	utils::rng.multinom(Rp, NQ, R);

}

void LazyMixtureSampler::updateMultParams(){
	for(qq=0;qq<NQ;++qq){
		if(qq == R)
			memcpy(countsTmp,richDirParameters,NAT*sizeof(double));
		else
			memcpy(countsTmp,poorDirParameters,NAT*sizeof(double));
		for(uint at=0;at<NAT;++at)
			countsTmp[at] += counts[qq][at];

		utils::sampleDirichlet(multParams[qq], countsTmp,NAT);
	}
}

//Sample state attribute (AT) only when necessary
uint LazyMixtureSampler::getAT(uint xx, uint yy){

	if(ATtmp[xx][yy] == -1){
		if(ATobserved[xx][yy] == -1){ //if unobserved (note: change of coordinates from agent to world coords
			//Find quadrant
			if(xx < XGhalf){
				if(yy < YGhalf)
					qq = 0;
				else
					qq = 1;
			}
			else{
				if(yy < YGhalf)
					qq = 2;
				else
					qq = 3;
			}
			uint samp;
			utils::rng.multinom(multParams[qq],NAT,samp);		
			ATtmp[xx][yy] = samp;
			return samp;
		}
		else
			return ATobserved[xx][yy];
	}
	else
		return ATtmp[xx][yy];

}

SIMULATOR2D* LazyMixtureSampler::updateMDPSample_ind(){
	for(uint b=0;b<2;++b){	
		updateR();
		updateMultParams(); 
	}
	return updateMDPSample();
}
SIMULATOR2D* LazyMixtureSampler::updateMDPSample(){

	//Do one pass of Gibb sampling
	updateR();
	updateMultParams(); 

	//Reset unobserved AT
	for(uint xx=0;xx<XG+2;++xx){
		std::fill(ATtmp[xx],ATtmp[xx]+YG+2,-1);	
	}	
	return (SIMULATOR2D*) simulator;	
}


void LazyMixtureSampler::render(double startx, double starty, double sqsz){
#if defined(GX)
	for(uint xx=0; xx < XG+2; xx++)
	{
		for(uint yy=0; yy < YG+2; yy++)
		{
			int ii = ATobserved[xx][yy];
			int jj = ATtmp[xx][yy];
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
			else if(jj >=0){
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
#endif
}
