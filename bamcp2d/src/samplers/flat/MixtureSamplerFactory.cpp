#include "MixtureSamplerFactory.h"

#include "MixtureSampler.h"
#include "LazyMixtureSampler.h"

#if defined(GX)	
	#include <GL/glfw.h>
#endif

MixtureSamplerFactory::MixtureSamplerFactory(bool _lazy,int _mode) : lazy(_lazy), mode(_mode){

	STARTX = 5; STARTY = 2;
	STARTXP = 0; STARTYP = 0;
	if(mode == 0){
		NAT = 3;
	}else{
		NAT = 4;
	}
	ATobserved = new int*[XG+2]; //+2 for the boundaries - FIX THIS
	for(uint xx=0;xx<XG+2;++xx){
		ATobserved[xx] = new int[YG+2];
		std::fill(ATobserved[xx],ATobserved[xx]+YG+2,-1);
		//+ fill up world boundaries FIXME assumes 2 is wall
		ATobserved[xx][0] = 2;
		ATobserved[xx][YG+1] = 2;
	}
	for(uint yy =0;yy<YG+2;++yy){
		ATobserved[0][yy] = 2;
		ATobserved[XG+1][yy] = 2;
	}

	if(mode == 2){ //Draw cross mask
		for(uint xx=7; xx < 15; ++xx){
			for(uint yy = 7; yy < 15; ++yy){
				ATobserved[xx][yy] = 0;
			}
		}
		STARTX = 10;
		STARTY = 10;
		STARTXP = 10; //10;
		STARTYP = 5; //5;
	}
	
	counts = new uint*[NQ];
	for(uint ii=0;ii<NQ;++ii){
		counts[ii] = new uint[NAT];
		std::fill(counts[ii],counts[ii]+NAT,0);
	}

	//For rendering -------------------
	ATcolor = new double*[NAT];
	for(uint ii=0;ii<NAT;++ii){
		ATcolor[ii] = new double[3];
		ATcolor[ii][0] = 0.5;
		ATcolor[ii][1] = 0.5;
		ATcolor[ii][2] = 0.5;
	}
	ATcolor[0][0] = 48.0/255; ATcolor[0][1] = 48.0/255; ATcolor[0][2] = 82.0/255;
	ATcolor[1][0] = 1.0; ATcolor[1][1] = 1.0; ATcolor[1][2] = 0.0;
	ATcolor[2][0] = 0.0; ATcolor[2][1] = 0.0; ATcolor[2][2] = 25.0/255;
	if(NAT>3){
		ATcolor[3][0] = 0.1; ATcolor[3][1] = 0.8; ATcolor[3][2] = 0.1;}
	//---------------------------------
	
}
SamplerFactory2D* MixtureSamplerFactory::createNewClone(){
	SamplerFactory2D* clone;
	clone = new MixtureSamplerFactory(lazy,mode);
	return clone;
}
void MixtureSamplerFactory::Clone(SamplerFactory2D* parent){
	NAT = ((MixtureSamplerFactory*)parent)->NAT;
	lazy = ((MixtureSamplerFactory*)parent)->lazy;
	mode = ((MixtureSamplerFactory*)parent)->mode;
	//Copy ATobserved 
	for(uint x=0;x<XG+2;++x)
		memcpy(ATobserved[x],((MixtureSamplerFactory*)parent)->ATobserved[x],(YG+2)*sizeof(int));
	for(uint nq=0;nq<NQ;++nq) 
		memcpy(counts[nq],((MixtureSamplerFactory*)parent)->counts[nq],NAT*sizeof(uint));

}

Sampler2D* MixtureSamplerFactory::getMDPSampler(double gamma){
	Sampler2D* sampler;
	if(lazy)
		sampler = new LazyMixtureSampler(gamma,NAT,ATobserved,counts,STARTX,STARTY,STARTXP,STARTYP);
	else
		sampler = new MixtureSampler(gamma,NAT,ATobserved,counts,STARTX,STARTY,STARTXP,STARTYP);
	return sampler;
}
void MixtureSamplerFactory::reset(){
	for(uint xx=0;xx<XG+2;++xx){
		std::fill(ATobserved[xx],ATobserved[xx]+YG+2,-1);
		ATobserved[xx][0] = 2;
		ATobserved[xx][YG+1] = 2;
	}
	for(uint yy =0;yy<YG+2;++yy){
		ATobserved[0][yy] = 2;
		ATobserved[XG+1][yy] = 2;
	}
	if(mode == 2){ //Draw cross mask
		for(uint xx=7; xx < 15; ++xx){
			for(uint yy = 7; yy < 15; ++yy){
				ATobserved[xx][yy] = 0;
			}
		}
		STARTX = 10;
		STARTY = 10;
		STARTXP = 10; //10;
		STARTYP = 5; //5;
	}
	
	for(uint ii=0;ii<NQ;++ii){
		for(uint jj=0;jj<NAT;++jj)
			counts[ii][jj] = 0;
	}
}

MixtureSamplerFactory::~MixtureSamplerFactory(){
	for(uint xx=0;xx<XG+2;++xx)
		delete[] ATobserved[xx];
	delete[] ATobserved;
	for(uint ii=0;ii<NQ;++ii)
		delete[] counts[ii];
	delete[] counts;

}


bool onBoundary(uint x,uint y){
	if(x == 0 || y == 0 || x == XG+1 || y == YG+1)
		return true;
	else
		return false;
}
uint quadrant(uint x, uint y){
	if(x < XG/2+1){
		if(y < YG/2+1)
			return 0;
		else
			return 1;
	}
	else{
		if(y < YG/2+1)
			return 2;
		else
			return 3;
	}
}

void MixtureSamplerFactory::extractObs(State2D* state){
	uint x = state->x+1; //Agent to world coordinates
	uint y = state->y+1;
	//For each AT component, look if already observed
	
	if(ATobserved[x+1][y+1] == -1){
		ATobserved[x+1][y+1] = state->AT[8];
		if(!onBoundary(x+1,y+1))
			counts[quadrant(x+1,y+1)][state->AT[8]]++;
	}
	if(ATobserved[x+1][y] == -1){
		ATobserved[x+1][y] = state->AT[7];
		if(!onBoundary(x+1,y))
			counts[quadrant(x+1,y)][state->AT[7]]++;
	}
	if(ATobserved[x+1][y-1] == -1){
		ATobserved[x+1][y-1] = state->AT[6];
		if(!onBoundary(x+1,y-1))
			counts[quadrant(x+1,y-1)][state->AT[6]]++;
	}
	if(ATobserved[x][y-1] == -1){
		ATobserved[x][y-1] = state->AT[5];
		if(!onBoundary(x,y-1))
			counts[quadrant(x,y-1)][state->AT[5]]++;
	}
	
	if(ATobserved[x][y] == -1){
		ATobserved[x][y] = state->AT[4];
		if(!onBoundary(x,y))
			counts[quadrant(x,y)][state->AT[4]]++;
	}
	if(ATobserved[x][y+1] == -1){
		ATobserved[x][y+1] = state->AT[3];
		if(!onBoundary(x,y+1))
			counts[quadrant(x,y+1)][state->AT[3]]++;
	}
	if(ATobserved[x-1][y+1] == -1){
		ATobserved[x-1][y+1] = state->AT[2];
		if(!onBoundary(x-1,y+1))
			counts[quadrant(x-1,y+1)][state->AT[2]]++;

	}
	if(ATobserved[x-1][y] == -1){
		ATobserved[x-1][y] = state->AT[1];
		if(!onBoundary(x-1,y))
			counts[quadrant(x-1,y)][state->AT[1]]++;
	}
	if(ATobserved[x-1][y-1] == -1){
		ATobserved[x-1][y-1] = state->AT[0];
		if(!onBoundary(x-1,y-1))
			counts[quadrant(x-1,y-1)][state->AT[0]]++;
	}
	
}
void MixtureSamplerFactory::firstObs(State2D* state){
	extractObs(state);
}
void MixtureSamplerFactory::update(State2D* state, uint a, State2D* obs){
	//Posterior update goes here	
	extractObs(obs);
}

void MixtureSamplerFactory::render(double startx, double starty, double sqsz){
#if defined(GX)	
	glColor3f(0.9,0.95,0.95);
	glPushMatrix();
	glTranslated(startx,starty+sqsz,0.0);
	glBegin(GL_QUADS);
	glVertex3f(0.0f, sqsz*YG, 0.0f);
	glVertex3f(sqsz*XG,sqsz*YG, 0.0f);
	glVertex3f(sqsz*XG,0.0f, 0.0f);
	glVertex3f(0.0f,0.0f, 0.0f);
	glEnd();
	glPopMatrix();

	for(uint xx=0; xx < XG+2; xx++)
	{
		for(uint yy=0; yy < YG+2; yy++)
		{
			int ii = ATobserved[xx][yy];
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
