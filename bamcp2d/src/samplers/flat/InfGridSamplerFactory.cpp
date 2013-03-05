#include "InfGridSamplerFactory.h"

#include "LazyInfGridSampler.h"

#if defined(GX)
	#include <GL/glfw.h>
#endif



InfGridSamplerFactory::InfGridSamplerFactory(double* _beta_params_col,
		double* _beta_params_row):
	beta_params_col(_beta_params_col), beta_params_row(_beta_params_row){
	
	NAT=2;
	STARTX = 500; STARTY = 500;
	lastx = STARTX;
	lasty = STARTY;
	lasta = 0;

	boundX = CHUNKX;
	boundY = CHUNKY;

	ATobserved = new int*[CHUNKX];
	for(uint xx=0;xx<CHUNKX;++xx){
		ATobserved[xx] = new int[CHUNKY];
		std::fill(ATobserved[xx],ATobserved[xx]+CHUNKY,-1);
	}
	PairObserved = new std::vector<Pair>();	
	PairObserved->reserve(CHUNKX);

	counts_col = new uint*[CHUNKX];
	for(uint xx=0;xx<CHUNKX;++xx){
		counts_col[xx] = new uint[2];
		counts_col[xx][0] = 0;
		counts_col[xx][1] = 0;
	}

	counts_row = new uint*[CHUNKY];
	for(uint yy=0;yy<CHUNKY;++yy){
		counts_row[yy] = new uint[2];
		counts_row[yy][0] = 0;
		counts_row[yy][1] = 0;
	}


	pcol = new double[boundX];
	prow = new double[boundY];
	std::fill(pcol,pcol+boundX,-1);
	std::fill(prow,prow+boundY,-1);


	//For rendering -------------------
	ATcolor = new double*[NAT];
	for(uint i=0;i<NAT;++i){
		ATcolor[i] = new double[3];
		ATcolor[i][0] = 0.5;
		ATcolor[i][1] = 0.5;
		ATcolor[i][2] = 0.5;
	}

	assert(NAT >= 2);
	ATcolor[0][0] = 48.0/255; ATcolor[0][1] = 48.0/255; ATcolor[0][2] = 82.0/255;
	ATcolor[1][0] = 0.1; ATcolor[1][1] = 0.8; ATcolor[1][2] = 0.1;

	//---------------------------------
	
#if defined(GX)
	pcol_mean = new double[22];
	std::fill(pcol_mean,pcol_mean+22,0);
	prow_mean = new double[22];
	std::fill(prow_mean,prow_mean+22,0);
	pcol_upcounts = new uint[22];
	std::fill(pcol_upcounts,pcol_upcounts+22,5);
	prow_upcounts = new uint[22];
	std::fill(prow_upcounts,prow_upcounts+22,5);

#endif
}

InfGridSamplerFactory::~InfGridSamplerFactory(){
	for(uint xx=0;xx<boundX;++xx)
		delete[] ATobserved[xx];
	delete[] ATobserved;
	

	for(uint xx=0;xx<CHUNKX;++xx)
		delete[] counts_col[xx];
	delete[] counts_col;
	
	for(uint yy=0;yy<CHUNKY;++yy)
		delete[] counts_row[yy];
	delete[] counts_row;
	
	delete[] pcol;
	delete[] prow;
	
	delete PairObserved;

#if defined(GX)
	delete[] pcol_mean;
	delete[] prow_mean;
	delete[] pcol_upcounts;
	delete[] prow_upcounts;
#endif

}


SamplerFactory2D* InfGridSamplerFactory::createNewClone(){
	SamplerFactory2D* clone;
	clone = new InfGridSamplerFactory(beta_params_col,beta_params_row);
	return clone;
}

void InfGridSamplerFactory::Clone(SamplerFactory2D* parent){
	//mode = ((InfGridSamplerFactory*)parent)->mode;
	
	//Copy ATobserved  
///	for(uint x=0;x<XG+2;++x)
///		memcpy(ATobserved[x],((InfGridSamplerFactory*)parent)->ATobserved[x],(YG+2)*sizeof(int));
///	for(uint nq=0;nq<NQ;++nq) 
///		memcpy(counts[nq],((InfGridSamplerFactory*)parent)->counts[nq],NAT*sizeof(uint));

	assert(0); //FIXME copy also rest of the variables

}

Sampler2D* InfGridSamplerFactory::getMDPSampler(double gamma){
	Sampler2D* sampler;
	sampler = new LazyInfGridSampler(gamma,ATobserved,counts_col,counts_row,PairObserved,boundX,boundY,STARTX,STARTY,beta_params_col,beta_params_row,pcol,prow);
	((LazyInfGridSampler*)sampler)->setLastPos(lastx,lasty);

#if defined(GX)	
	((LazyInfGridSampler*)sampler)->setLastPMeans(pcol_mean,prow_mean,pcol_upcounts,prow_upcounts,lasta);
#endif

	return sampler;
}
void InfGridSamplerFactory::reset(){
	for(uint xx=0;xx<boundX;++xx){
		std::fill(ATobserved[xx],ATobserved[xx]+boundY,-1);
		counts_col[xx][0] = 0;
		counts_col[xx][1] = 0;
	}
	for(uint yy=0;yy<boundY;++yy){
		counts_row[yy][0] = 0;
		counts_row[yy][1] = 0;
	}
	PairObserved->clear();	
	lastx = STARTX;
	lasty = STARTY;
	lasta = 0;
	std::fill(pcol,pcol+boundX,-1);
	std::fill(prow,prow+boundY,-1);
#if defined(GX)
	std::fill(pcol_mean,pcol_mean+22,0);
	std::fill(prow_mean,prow_mean+22,0);
	std::fill(pcol_upcounts,pcol_upcounts+22,5);
	std::fill(prow_upcounts,prow_upcounts+22,5);
#endif
}

void InfGridSamplerFactory::extractObs(State2D* state){
	if(ATobserved[state->x][state->y] == -1){ //Important since ATobserved can change from 1-->0 on second state visit
		ATobserved[state->x][state->y] = state->AT[0];
		if(state->AT[0] == 1){
			counts_row[state->y][0]++;  //success
			counts_col[state->x][0]++;
		}
		else{
			counts_row[state->y][1]++; // failure
			counts_col[state->x][1]++;
		}
		//FIXME init pcol[xx] && prow[yy] if < 0
		PairObserved->push_back(Pair(state->x,state->y));
	}

	assert(state->x > 1 && state->x < boundX - 1);
	assert(state->y > 1 && state->y < boundY - 1); //FIXME Implement adaptive grid size
}
void InfGridSamplerFactory::firstObs(State2D* state){
	extractObs(state);
}
void InfGridSamplerFactory::update(State2D* state, uint a, State2D* obs){
	//Posterior update goes here	
	extractObs(obs);
	
	lastx = obs->x;
	lasty = obs->y;
	lasta = a;
}

void InfGridSamplerFactory::render(double startx, double starty, double sqsz){
	
#if defined(GX)	
	uint S=40;
	sqsz = sqsz/2;

	glColor3f(0,0,0);
	glPushMatrix();
	glTranslated(startx,starty,0.0);
	glBegin(GL_QUADS);
	glVertex3f(0.0f, sqsz*S, 0.0f);
	glVertex3f(sqsz*S,sqsz*S, 0.0f);
	glVertex3f(sqsz*S,0.0f, 0.0f);
	glVertex3f(0.0f,0.0f, 0.0f);
	glEnd();
	glPopMatrix();

	for(uint xx=0; xx < S; xx++)
	{
		for(uint yy=0; yy < S; yy++)
		{
			int ii = ATobserved[xx+lastx-20][yy+lasty-20];
			if(ii>=0){
				glColor3f(ATcolor[ii][0],ATcolor[ii][1],ATcolor[ii][2]+utils::rng.rand_closed01()/15);
				glPushMatrix();
				glTranslated(startx+(xx-1)*sqsz,starty+yy*sqsz,0.0);
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
