#include "lazyInfGrid.h"
#include "utils/utils.h"

#include "samplers/flat/LazyInfGridSampler.h"
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

LazyInfGrid::LazyInfGrid(double discount, LazyInfGridSampler* _sampler): sampler(_sampler)
{
		A = 4;
		NumAttributes = 2;
		NumActions = A;
		NumC = 1;
		
		STARTX = 2;
		STARTY = 2;

		Discount = discount;

    RewardRange = 1;
    
    //Size of grid in memory
		boundX = 1000;
		boundY = 1000;

		ATcolor = new double*[NumAttributes];
		for(uint i=0;i<NumAttributes;++i){
			ATcolor[i] = new double[3];
			ATcolor[i][0] = 0.5;
			ATcolor[i][1] = 0.5;
			ATcolor[i][2] = 0.5;
		}

		assert(NumAttributes >= 2);
		ATcolor[0][0] = 48.0/255; ATcolor[0][1] = 48.0/255; ATcolor[0][2] = 82.0/255;
		ATcolor[1][0] = 0.1; ATcolor[1][1] = 0.8; ATcolor[1][2] = 0.1;
		
}


LazyInfGrid::~LazyInfGrid(){
	for(uint i=0;i<NumAttributes;++i)
		delete[] ATcolor[i];
	delete[] ATcolor;
}

void LazyInfGrid::reset(){
}

void LazyInfGrid::Save(std::string filename){
	assert(0);
}
void LazyInfGrid::Load(std::string filename){
	assert(0);
}

void LazyInfGrid::render(double startx, double starty, double sqsz) const{
	sampler->render(startx,starty,sqsz);
}

void LazyInfGrid::CreateStartState(State2D* state) const
{
	assert(0);
}

//Can always move in any direction
bool LazyInfGrid::move(State2D* state, uint action) const{
	return true; 
}

uint LazyInfGrid::getATxy(uint x, uint y) const{
	assert(0); 
	return 0;
}

bool LazyInfGrid::Step(State2D* state, uint action, 
    State2D* observation, double& reward)
{
	assert(state->x > 1 && state->x < boundX -1);
	assert(state->y > 1 && state->y < boundY -1);
	//FIXME implement adaptive XYvisited size
	
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
	
	observation->AT[0] = sampler->getAT(observation->x,observation->y);
	reward = state->AT[0];
	
	return false; //not episodic
}
	
uint LazyInfGrid::getRLPI(uint**& RLPI, State2D* state, double Timeout) const{
	std::cout << "does not compute :(" << std::endl;
	assert(0);	
}

void LazyInfGrid::deleteRLPI(uint**& RLPI) const{
	std::cout << "does not compute :(" << std::endl;
	assert(0);	
}

