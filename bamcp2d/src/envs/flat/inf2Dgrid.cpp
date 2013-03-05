#include "inf2Dgrid.h"
#include "utils/utils.h"
#include <algorithm>

#if defined(GX)
	#include <GL/glfw.h>
#endif

#include "boost/timer.hpp"

#include <fstream>

using namespace UTILS;

// Infinite binary grid with reward distributed according to Bernoulli(pi*pj) where 
// pi ~ Beta(alpha_row,beta_row) for each row i
// pj ~ Beta(alpha_col,alpha_col) for each column j


// Vision + 0 around grid location (=> state includes 1 attribute)
//      _
//     |X|
//      

InfGrid::InfGrid(double discount,
		double* _beta_params_col,
		double* _beta_params_row) :
	beta_params_col(_beta_params_col),
	beta_params_row(_beta_params_row)
{
		A = 4;
		NumAttributes = 2;
		NumActions = A;
		NumC = 1;
		
		STARTX = 500;
		STARTY = 500;
		
		lastx = STARTX;
		lasty = STARTY;

		Discount = discount;

    RewardRange = 1;
  

    //Current size of grid in memory
		boundX = 1000;
		boundY = 1000;
		
		//Initialize binary ATmap (with a moving boundary)
		ATmap = new uint*[boundX];
		map = new uint*[boundX];
		for(uint x=0;x<boundX;++x){
			ATmap[x] = new uint[boundY];
			map[x] = new uint[boundY];
			std::fill(ATmap[x],ATmap[x]+boundY,0);
		}
		
		//Draw row/column p parameter
		pcol = new double[boundX];
		prow = new double[boundY];
		double* sampled_param= new double[2];
		
		for(uint y=0;y<boundY;++y){
			utils::sampleDirichlet(sampled_param,beta_params_row,2);
			prow[y] = sampled_param[0];
		}
		for(uint x=0;x<boundX;++x){
			utils::sampleDirichlet(sampled_param,beta_params_col,2);
			pcol[x] = sampled_param[0];
		}

		delete[] sampled_param;
	
		//Draw reward ATmap
		for(uint x=0;x<boundX;++x){
			for(uint y=0;y<boundY;++y){
				if(utils::rng.rand_closed01() < pcol[x]*prow[y])
					ATmap[x][y] = 1;
			}
		}
		
		for(uint x=0;x<boundX;++x)
			memcpy(map[x],ATmap[x],boundY*sizeof(uint));
		
		ATcolor = new double*[NumAttributes+1];
		for(uint i=0;i<NumAttributes+1;++i){
			ATcolor[i] = new double[3];
			ATcolor[i][0] = 0.5;
			ATcolor[i][1] = 0.5;
			ATcolor[i][2] = 0.5;
		}

		assert(NumAttributes >= 2);
		ATcolor[0][0] = 48.0/255; ATcolor[0][1] = 48.0/255; ATcolor[0][2] = 82.0/255;
		ATcolor[1][0] = 0.1; ATcolor[1][1] = 0.8; ATcolor[1][2] = 0.1;
		ATcolor[1][0] = 0.1; ATcolor[1][1] = 0.5; ATcolor[1][2] = 0.1;
		
}



InfGrid::~InfGrid(){
	delete[] prow;
	delete[] pcol;

	for(uint i=0;i<boundX;++i){
		delete[] ATmap[i];
		delete[] map[i];
	}
	delete[] ATmap;
	delete[] map;
	
	for(uint i=0;i<NumAttributes;++i)
		delete[] ATcolor[i];
	delete[] ATcolor;
}

void InfGrid::reset(){
	for(uint x=0;x<boundX;++x)
		memcpy(map[x],ATmap[x],boundY*sizeof(uint));
}

void InfGrid::Save(std::string filename){
	std::ofstream ofs;
	ofs.open(filename.c_str());
	ofs << boundX << " " << boundY << std::endl;
	ofs << STARTX << " " << STARTY << std::endl;
	uint xx=0;
	for(;xx<boundX;++xx){
		for(uint yy=0;yy<boundY;++yy){
			ofs << ATmap[xx][yy] << " ";
		}
		ofs << std::endl;
	}
	ofs.close();

}
void InfGrid::Load(std::string filename){
	
	for(uint i=0;i<boundX;++i){
		delete[] ATmap[i];
		delete[] map[i];
	}
	delete[] ATmap;

	std::ifstream ifs;
	ifs.open(filename.c_str());
	assert(ifs.good());
	ifs >> boundX; ifs >> boundY; 
	ifs >> STARTX; ifs >> STARTY;

	ATmap = new uint*[boundX];
	map = new uint*[boundX];
	for(uint x=0;x<boundX;++x){
		ATmap[x] = new uint[boundY];
		map[x] = new uint[boundY];
	}
	
	for(uint xx =0;xx<boundX;++xx){
		for(uint yy=0;yy<boundY;++yy)
			ifs >> ATmap[xx][yy];
	}
	ifs.close();

	reset();
	
}

void InfGrid::render(double startx, double starty, double sqsz) const{
// Draw window of fixed size around agent from last seen agent's position

#if defined(GX)	
	for(uint xx=0; xx < 20; xx++)
	{
		for(uint yy=0; yy < 20; yy++)
		{
			uint ii = map[xx+lastx-10][yy+lasty-10];
			
			glColor3f(ATcolor[ii][0],ATcolor[ii][1],ATcolor[ii][2]+utils::rng.rand_closed01()/15);
			if(ii != ATmap[xx+lastx-10][yy+lasty-10])
				glColor3f(ATcolor[2][0],ATcolor[2][1],ATcolor[2][2]+utils::rng.rand_closed01()/15);

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
void InfGrid::renderMAPXYOffset(double& offsetx,double& offsety) const{
	offsetx = -lastx+9;
	offsety = -lasty+9;
}
void InfGrid::CreateStartState(State2D* state) const
{
	state->x = STARTX;
	state->y = STARTY;

	state->x2 = 0;
	state->y2 = 0;
	state->wait = -1;
	
	state->AT[0] = map[STARTX][STARTY];
}

	//Action ATmapping
	//       0
	//       ^
	//       |
	//  3 <-- --> 1
	//       |
	//       v
	//       2

// Grid:
//     .
//   | .
//   | ...
//   |____
//  0,0

//Can always move in any direction
bool InfGrid::move(State2D* state, uint action) const{
	return true; 
}

uint InfGrid::getATxy(uint x, uint y) const{
	assert(0); // Is it ATmap or map that we want to check?
	//FIXME check bounds
	return ATmap[x][y];
}

bool InfGrid::Step(State2D* state, uint action, 
    State2D* observation, double& reward)
{
	assert(state->x > 1 && state->x < boundX -1);
	assert(state->y > 1 && state->y < boundY -1);
	//FIXME implement adaptive ATmap size
  //Increase size here
	
	if(action==0){
		observation->x = state->x;
		observation->y = state->y + 1;
	}
	else if(action==1){
		observation->x = state->x+1;
		observation->y = state->y;
	}
	else if(action==2){
		observation->x = state->x;
		observation->y = state->y - 1;
	}
	else{
		observation->x = state->x - 1;
		observation->y = state->y;
	}
	observation->AT[0] = map[observation->x][observation->y];
	reward = map[state->x][state->y]; //Reward from leaving state 
	map[state->x][state->y] = 0; //Reward is consumed
	
	lastx = observation->x;
	lasty = observation->y;

	return false; //not episodic
}
	
uint InfGrid::getRLPI(uint**& RLPI, State2D* state, double Timeout) const{
	std::cout << "does not compute :(" << std::endl;
	assert(0);	
}

void InfGrid::deleteRLPI(uint**& RLPI) const{
	std::cout << "does not compute :(" << std::endl;
	assert(0);	
}

