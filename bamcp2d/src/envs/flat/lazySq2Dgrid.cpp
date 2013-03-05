#include "sq2Dgrid.h"
#include "utils/utils.h"
#include <algorithm>
#include "samplers/flat/LazyMixtureSampler.h"


using namespace UTILS;

// Square grid environment with finite set of attributes per grid location

// Vision + 1 around grid location (=> state includes 9 attributes)
//      _ _ _
//     |_!_!_|
//     |_!X!_|
//     |_!_!_| 
//
// [BAMDPs derived from this formulation are a bit awkward, but it doesn't
// matter for simulation-based algorithms like BAMCP.]
//
//State2D.AT[]:
//
//		 2 3 8
//     1 4 7
//     0 5 6
//
// TODO: make surround vision size a parameter (hardcoded for now)

LazySqGrid::LazySqGrid(uint _X, uint _Y, uint _numAttributes, double discount,
		LazyMixtureSampler* _sampler)
:   X(_X+2), Y(_Y+2), sampler(_sampler)
{
		A = 4;
		NumAttributes = _numAttributes;
		NumActions = A;
		NumC = 9;

		STARTX = 2;
		STARTY = 2;
		STARTXP = 0;
		STARTYP = 0;

		Discount = discount;

    RewardRange = 1;
		//Hardcore cost/attribute here
		ATr = new double[NumAttributes];
		std::fill(ATr,ATr+NumAttributes,0);	
		assert(NumAttributes > 2);
		ATr[1] = 1.0; //AT 1 is "goal"
		moveCost = 0.0;
		PREDWAIT = -1.0;
		PREDCOST = 0.0;

		ATw = new bool[NumAttributes];
		std::fill(ATw,ATw+NumAttributes,false);
		ATw[2] = true; //AT 2 is "wall" 
		
		ATcolor = new double*[NumAttributes];
		for(uint i=0;i<NumAttributes;++i){
			ATcolor[i] = new double[3];
			ATcolor[i][0] = 0.5;
			ATcolor[i][1] = 0.5;
			ATcolor[i][2] = 0.5;
		}
		assert(NumAttributes >= 3);
		ATcolor[0][0] = 48.0/255; ATcolor[0][1] = 48.0/255; ATcolor[0][2] = 82.0/255;
		ATcolor[1][0] = 1.0; ATcolor[1][1] = 1.0; ATcolor[1][2] = 0.0;
		ATcolor[2][0] = 0.0; ATcolor[2][1] = 0.0; ATcolor[2][2] = 25.0/255;
		if(NumAttributes > 3){
			ATcolor[3][0] = 0.1; ATcolor[3][1] = 0.8; ATcolor[3][2] = 0.1;
		}
		
}

LazySqGrid::~LazySqGrid(){
	delete[] ATr;
	delete[] ATw;
	delete[] ATcolor;
}

void LazySqGrid::render(double startx, double starty, double sqsz) const{
	sampler->render(startx,starty,sqsz);
}

void LazySqGrid::setMoveCost(double _cost){
	moveCost = _cost;
}

void LazySqGrid::setPREDVARS(double _PREDCOST, double _PREDWAIT){
	PREDCOST = _PREDCOST;
	PREDWAIT = _PREDWAIT;
}

void LazySqGrid::setATr(uint AT, double r){
	ATr[AT] = r;	
}
void LazySqGrid::setATxy(uint AT, uint x, uint y){
	//Only sampler sets AT
}
uint LazySqGrid::getATxy(uint x, uint y) const{
	return 0;
}

void LazySqGrid::CreateStartState(State2D* state) const
{
	state->x = STARTX;
	state->y = STARTY;
	if(PREDWAIT >= 0){
		state->x2 = STARTXP;
		state->y2 = STARTYP;
		state->wait = 0;
	}
	else{
		state->x2 = 0;
		state->y2 = 0;
		state->wait = -1;
	}
	setATObs(state,state->x,state->y);
}

	//Action mapping
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


inline uint LazySqGrid::getCoord(uint x, uint y) const{
	return (x+1)*Y+(y+1);
}
void LazySqGrid::getXY(uint coord, uint& x, uint& y) const{
	assert(0); //not tested and converts to internal coordinate system
	x = coord/Y;
	y = coord - x*Y;
}
//Set the state attributes (attributes around state position) to corresponding
//values in ATmap
// observation:  state to be updated
// x-y *world* coordinates
//		 2 3 8
//     1 4 7
//     0 5 6
void LazySqGrid::setATObs(State2D* observation,uint xx, uint yy) const{
	observation->AT[0] = sampler->getAT(xx,yy); yy++;
	observation->AT[1] = sampler->getAT(xx,yy); yy++;
	observation->AT[2] = sampler->getAT(xx,yy); xx++;
	observation->AT[3] = sampler->getAT(xx,yy); yy--;
	observation->AT[4] = sampler->getAT(xx,yy); yy--;
	observation->AT[5] = sampler->getAT(xx,yy); xx++;
	observation->AT[6] = sampler->getAT(xx,yy); yy++;
	observation->AT[7] = sampler->getAT(xx,yy); yy++;
	observation->AT[8] = sampler->getAT(xx,yy);
}

inline int sign(int val){
  return (val > 0) - (val < 0);
}

bool LazySqGrid::move(State2D* state, uint action) const{
	if(action == 0 && state->y < Y-1 && !ATw[sampler->getAT(state->x+1,state->y+2)]){ //moving up?
		return true;	
	}
	else if(action == 2 && state->y > 0 && !ATw[sampler->getAT(state->x+1,state->y)]){ //moving south?
		return true;	
	}
	else if(action == 1 && state->x < X-1 && !ATw[sampler->getAT(state->x+2,state->y+1)]){ //moving east?
		return true;	
	}
	else if(action == 3 && state->x > 0 && !ATw[sampler->getAT(state->x,state->y+1)]){ //moving west?
		return true;
	}
	else
		return false;
}
bool LazySqGrid::Step(State2D* state, uint action, 
    State2D* observation, double& reward)
{
	reward = 0;
	uint worldSx = state->x+1;
	uint worldSy = state->y+1;
	bool move = false;
	if(action == 0){
		if(state->y < Y-1 && !ATw[sampler->getAT(worldSx,worldSy+1)]){ //moving up?
			observation->x = state->x;
			observation->y = state->y+1;
			setATObs(observation,observation->x,observation->y);	
			reward -= moveCost;
			move = true;
		}
	}
	else if(action == 2){
		if(state->y > 0 && !ATw[sampler->getAT(worldSx,worldSy-1)]){ //moving south?
			observation->x = state->x;
			observation->y = state->y-1;
			setATObs(observation,observation->x,observation->y);
			reward -= moveCost;
			move = true;
		}
	}
	else if(action == 1){
		if(state->x < X-1 && !ATw[sampler->getAT(worldSx+1,worldSy)]){ //moving east?
			observation->x = state->x+1;
			observation->y = state->y;
			setATObs(observation,observation->x,observation->y);
			reward -= moveCost;
			move = true;
		}
	}
	else if(action == 3){
		if(state->x > 0 && !ATw[sampler->getAT(worldSx-1,worldSy)]){ //moving west?
			observation->x = state->x-1;
			observation->y = state->y;
			setATObs(observation,observation->x,observation->y);
			reward -= moveCost;
			move = true;
		}
	}
	
	if(!move){ //not moving	
		observation->x = state->x;
		observation->y = state->y;
		memcpy(observation->AT,state->AT,NumC*sizeof(uint));
	}


	reward += ATr[state->AT[4]]; //reward for current state position

	if(PREDWAIT > -1){ //Predator scenario ?
		//Handle predator movement and cost
		//Deterministic predator movement
		int offsetx = sign(state->x - state->x2);
		int offsety = sign(state->y - state->y2);
		observation->wait = state->wait;
		if(state->wait==0){
			observation->x2=state->x2+offsetx;
			observation->y2=state->y2+offsety;

			observation->wait = PREDWAIT;
		}
		else{
			observation->x2=state->x2;
			observation->y2=state->y2;
			observation->wait--;
		}
		if(abs(state->x - state->x2) < 2 && abs(state->y - state->y2) < 2)
			reward -= PREDCOST;
	}
	return false; //not episodic
}


