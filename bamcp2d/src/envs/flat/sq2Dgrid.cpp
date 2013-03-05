#include "sq2Dgrid.h"
#include "utils/utils.h"
#include <algorithm>

#if defined(GX)
	#include <GL/glfw.h>
#endif

#include "boost/timer.hpp"

#include <fstream>

using namespace UTILS;

// Square grid environment with finite set of attributes per grid location

// Vision + 1 around grid location (=> state includes 9 attributes)
//      _ _ 
//     |_!_!_|
//     |_!X!_|
//     |_!_!_| 
//
//State2D.AT[]:
//
//		 2 3 8
//     1 4 7
//     0 5 6
//
// TODO: make surround vision size a parameter (hardcoded for now)

SqGrid::SqGrid(uint _X, uint _Y, uint _numAttributes, double discount)
:   X(_X+2), Y(_Y+2)
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

		ATmap = new uint[X*Y];
		std::fill(ATmap,ATmap+X*Y,0);

		ATw = new bool[NumAttributes];
		std::fill(ATw,ATw+NumAttributes,false);
		ATw[2] = true; //AT 2 is "wall" 
		
		//wall around grid
		for(uint yy=0;yy<Y;++yy){
			ATmap[yy] = 2;
			ATmap[(X-1)*Y+yy] = 2;
		}
		for(uint xx=0;xx<X;++xx){
			ATmap[xx*Y] = 2;
			ATmap[(xx)*Y+(Y-1)] = 2;
		}
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

SqGrid::~SqGrid(){
	delete[] ATr;
	delete[] ATmap;
	delete[] ATw;

	for(uint i=0;i<NumAttributes;++i)
		delete[] ATcolor[i];
	delete[] ATcolor;
	
}

void SqGrid::Save(std::string filename){
	std::ofstream ofs;
	ofs.open(filename.c_str());
	ofs << X << " " << Y << " " << NumAttributes << " " << NumC << std::endl;
	ofs << moveCost << " " << PREDCOST << " " << PREDWAIT << std::endl;
	ofs << STARTX << " " << STARTY << std::endl;
	ofs << STARTXP << " " << STARTYP << std::endl;

	uint i =0;
	for(i=0;i<NumAttributes;++i)
		ofs << ATr[i] << " ";
	ofs << std::endl;
	for(i=0;i<NumAttributes;++i)
		ofs << ATw[i] << " ";
	ofs << std::endl;
	for(i=0;i<X*Y;++i)
		ofs << ATmap[i] << " ";
	ofs << std::endl;
	ofs.close();
}
void SqGrid::Load(std::string filename){
	delete[] ATr;
	delete[] ATmap;
	delete[] ATw;

	std::ifstream ifs;
	ifs.open(filename.c_str());
	assert(ifs.good());
	ifs >> X; ifs >> Y; ifs >> NumAttributes; ifs >> NumC;
	ifs >> moveCost; ifs >> PREDCOST; ifs >> PREDWAIT;
	ifs >> STARTX; ifs >> STARTY;
	ifs >> STARTXP; ifs >> STARTYP;

	ATr = new double[NumAttributes];
	ATmap = new uint[X*Y];
	ATw = new bool[NumAttributes];

	uint i =0;
	for(i=0;i<NumAttributes;++i)
		ifs >> ATr[i];
	for(i=0;i<NumAttributes;++i)
		ifs >> ATw[i];
	for(i=0;i<X*Y;++i)
		ifs >> ATmap[i];
	ifs.close();
}

/*
void SqGrid::setATmap(uint* _ATmap){
	delete[] ATmap;
	ATmap = _ATmap;
}*/

void SqGrid::render(double startx, double starty, double sqsz) const{
#if defined(GX)	
	for(uint xx=0; xx < X; xx++)
	{
		for(uint yy=0; yy < Y; yy++)
		{
			uint ii = ATmap[xx*Y+yy];
			glColor3f(ATcolor[ii][0],ATcolor[ii][1],ATcolor[ii][2]+utils::rng.rand_closed01()/15);

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

void SqGrid::setMoveCost(double _cost){
	moveCost = _cost;
}
void SqGrid::setPREDVARS(double _PREDCOST, double _PREDWAIT){
	PREDCOST = _PREDCOST;
	PREDWAIT = _PREDWAIT;
}

void SqGrid::setATr(uint AT, double r){
	ATr[AT] = r;	
}
void SqGrid::setATxy(uint AT, uint x, uint y){
	ATmap[(x+1)*Y+(y+1)] = AT; //Add 1 to account for wall
}
uint SqGrid::getATxy(uint x, uint y) const{
	return ATmap[(x+1)*Y+(y+1)];
}

bool SqGrid::checkLegalStart(){
	if(ATw[ATmap[getCoord(STARTX,STARTY)]])
		return false;
	else
		return true;
}
void SqGrid::CreateStartState(State2D* state) const
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
	uint coord = getCoord(state->x-1,state->y-1);
	setATObs(state,coord);
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


inline uint SqGrid::getCoord(uint x, uint y) const{
	return (x+1)*Y+(y+1);
}
void SqGrid::getXY(uint coord, uint& x, uint& y) const{
	assert(0); //not tested and converts to internal coordinate system
	x = coord/Y;
	y = coord - x*Y;
}
//Set the state attributes (attributes around state position) to corresponding
//values in ATmap
// observation:  state to be updated
// coord:        lower left attribute coordinate
//		 2 3 8
//     1 4 7
//     0 5 6
void SqGrid::setATObs(State2D* observation,uint coord) const{
	observation->AT[0] = ATmap[coord]; coord++;
	observation->AT[1] = ATmap[coord]; coord++;
	observation->AT[2] = ATmap[coord]; coord += Y;
	observation->AT[3] = ATmap[coord]; coord--;
	observation->AT[4] = ATmap[coord]; coord--;
	observation->AT[5] = ATmap[coord]; coord += Y;
	observation->AT[6] = ATmap[coord]; coord++;
	observation->AT[7] = ATmap[coord]; coord++;
	observation->AT[8] = ATmap[coord];
}

inline int sign(int val){
  return (val > 0) - (val < 0);
}

bool SqGrid::move(State2D* state, uint action) const{
	if(action == 0 && state->y < Y-1 && !ATw[ATmap[getCoord(state->x,state->y+1)]]){ //moving up?
		return true;	
	}
	else if(action == 2 && state->y > 0 && !ATw[ATmap[getCoord(state->x,state->y-1)]]){ //moving south?
		return true;	
	}
	else if(action == 1 && state->x < X-1 && !ATw[ATmap[getCoord(state->x+1,state->y)]]){ //moving east?
		return true;	
	}
	else if(action == 3 && state->x > 0 && !ATw[ATmap[getCoord(state->x-1,state->y)]]){ //moving west?
		return true;
	}
	else
		return false;
}
bool SqGrid::Step(State2D* state, uint action, 
    State2D* observation, double& reward) 
{
	reward = 0;
	if(action == 0 && state->y < Y-1 && !ATw[ATmap[getCoord(state->x,state->y+1)]]){ //moving up?
		observation->x = state->x;
		observation->y = state->y+1;
		setATObs(observation,getCoord(observation->x-1,observation->y-1));	
		reward -= moveCost;
	}
	else if(action == 2 && state->y > 0 && !ATw[ATmap[getCoord(state->x,state->y-1)]]){ //moving south?
		observation->x = state->x;
		observation->y = state->y-1;
		setATObs(observation,getCoord(observation->x-1,observation->y-1));
		reward -= moveCost;
	}
	else if(action == 1 && state->x < X-1 && !ATw[ATmap[getCoord(state->x+1,state->y)]]){ //moving east?
		observation->x = state->x+1;
		observation->y = state->y;
		setATObs(observation,getCoord(observation->x-1,observation->y-1));
		reward -= moveCost;
	}
	else if(action == 3 && state->x > 0 && !ATw[ATmap[getCoord(state->x-1,state->y)]]){ //moving west?
		observation->x = state->x-1;
		observation->y = state->y;
		setATObs(observation,getCoord(observation->x-1,observation->y-1));
		reward -= moveCost;
	}
	else{ //not moving	
		observation->x = state->x;
		observation->y = state->y;
		memcpy(observation->AT,state->AT,NumC*sizeof(uint));
	}


	reward += ATr[state->AT[4]]; //reward for current state position

	if(PREDWAIT >= 0) // Predator scenario ?
	{
		//Handle predator movement and cost
		//Deterministic predator movement
		observation->wait = state->wait;
		if(state->wait==0){
			int offsetx = sign(state->x - state->x2);
			int offsety = sign(state->y - state->y2);
			observation->x2=state->x2+offsetx;
			observation->y2=state->y2+offsety;

			//Warning: making the predator movement depend on AT => more complex planning/inference problem
			//			if(ATw[ATmap[getCoord(observation->x2,observation->y2)]])
			//				observation->wait = 2;	
			//			else
			observation->wait = PREDWAIT;
		}
		else{
			observation->x2=state->x2;
			observation->y2=state->y2;
			observation->wait--;
		}
		if(abs(state->x - state->x2) < 2 && abs(state->y - state->y2) < 2){
			reward -= PREDCOST;
			//return true;
		}
	}
	return false; //not episodic
}
	

//Step without observations etc. (only for MDP solving)
void SqGrid::StepShort(uint pxy, uint xy, uint action, double& reward, uint& pxy2, uint& xy2) const{
	reward = 0;
	if(ATw[ATmap[xy]]){ //On boundary or on obstacle: nothing happens
		xy2 = xy;
	}
	else if(action == 0 && !ATw[ATmap[xy+1]]){ //moving up?
		xy2 = xy + 1;	reward -= moveCost;
	}
	else if(action == 2 && !ATw[ATmap[xy-1]]){ //moving south?
		xy2 = xy - 1;	reward -= moveCost;
	}
	else if(action == 1 && !ATw[ATmap[xy+Y]]){ //moving east?
		xy2=xy+Y; reward -= moveCost;
	}
	else if(action == 3 && !ATw[ATmap[xy-Y]]){ //moving west?
		xy2 = xy-Y;	reward -= moveCost;
	}
	else{ //not moving	
		xy2 = xy;	
	}
	reward += ATr[ATmap[xy]]; //reward for current state position

	if(PREDWAIT >= 0) // Predator scenario ?
	{
		assert(PREDWAIT > 0); 
		//Extract px,py,pwait (Probably a bit wasteful but we do it only once)
		int pXY = pxy/(PREDWAIT+1);
		int pwait = pxy % (PREDWAIT+1);
		
		int x = xy/Y;
		int px = pXY/Y;
		int xdiff = x - px;
		int ydiff = (xy-x*Y) - (pXY-px*Y);

		
		if(pwait==0){ //Shift predator position and adjust wait
			int offsetx = sign(xdiff);
			int offsety = sign(ydiff);
			pxy2 = (pXY+Y*(offsetx)+offsety)*(PREDWAIT+1); 
			pxy2 = pxy2 + PREDWAIT;
		}
		else{
			pxy2=pxy-1; //wait = wait - 1, same predator position
		}
		if(abs(xdiff) < 2 && abs(ydiff) < 2)
			reward -= PREDCOST;
	}
}

uint SqGrid::getRLPI(uint**& RLPI, State2D* state, double Timeout) const{
	if(!RLPI)
		solveVI(RLPI,Timeout);
	if(PREDWAIT > -1)
		return RLPI[((state->x2+1)*Y+(state->y2+1))*(PREDWAIT+1)+state->wait][(state->x+1)*Y+(state->y+1)];
	else
		return RLPI[0][(state->x+1)*Y+(state->y+1)];
}
void SqGrid::deleteRLPI(uint**& RLPI) const{
	if(RLPI){
		if(PREDWAIT >= 0){
			for(uint i=0;i<X*Y;++i)
				delete[] RLPI[i];
		}
		else
			delete[] RLPI[0];
		delete[] RLPI;
	}
}

//Use a sparse Value Iteration to solve the fully-observed MDP
void SqGrid::solveVI(uint**& RLPI, double Timeout) const{

	boost::timer timer;

	uint XY = X*Y;
	uint PXY;
	if(PREDWAIT >= 0)
		PXY = XY*(PREDWAIT+1);
	else
		PXY = 1;
	
	double epsilon = 0.001;
	double sqeps = epsilon*epsilon;

	//Initialize value function	
	double** V0;
	double** V1;	
	V0 = new double*[PXY];
	V1 = new double*[PXY];
	for(uint i=0;i<PXY;++i){
		V0[i] = new double[XY];
		V1[i] = new double[XY];
		std::fill(V0[i],V0[i]+XY,0);
		std::fill(V1[i],V1[i]+XY,0);
	}
	//Initialize policy
	if(!RLPI){
		RLPI = new uint*[PXY];
		for(uint i=0;i<PXY;++i)
			RLPI[i] = new uint[XY];
	}

	//Compute transitions only once
	uint pxy2 = 0;
	uint xy2 = 0;
	double r;

	double*** R = new double**[PXY];
	uint*** P = new uint**[PXY];
	for(uint i=0;i<PXY;++i){
		R[i] = new double*[XY];
		P[i] = new uint*[XY];
		for(uint j=0;j<XY;++j){
			R[i][j] = new double[A]; 
			P[i][j] = new uint[2*A]; 
			for(uint aa=0;aa<A;++aa){
				StepShort(i,j,aa,r,pxy2,xy2);
				R[i][j][aa] = r;
				P[i][j][aa+aa] = pxy2; // aa*2+{0,1}
				P[i][j][aa+aa+1] = xy2;
			}
		}
	}

	//For swapping V0/V1 after each iteration
	bool z = true;
	double** Vv;
	double** Vu;
	//Temp variables
	uint pxy;
	uint xy;
		uint aa;
	double Qaa = 0;
	double sqnorm = 0;
	//Iterate
	do{	
		//swap Vv and Vu
		if(z){ Vu = V1; Vv = V0; z=false;}
		else{Vu = V0; Vv = V1; z=true;}

		for(pxy = 0; pxy < PXY; ++pxy){
			for(xy = 0; xy < XY; ++xy){
				RLPI[pxy][xy] = 0;
				Vv[pxy][xy]	= -std::numeric_limits<double>::infinity(); 	
				for(aa = 0; aa < A; ++aa){
					//StepShort(pxy,xy,aa,r,pxy2,xy2);
					pxy2 = P[pxy][xy][aa+aa];
					xy2 = P[pxy][xy][aa+aa+1];
					Qaa = R[pxy][xy][aa] + Discount*Vu[pxy2][xy2];
					if(Qaa > Vv[pxy][xy]){
						RLPI[pxy][xy] = aa;
						Vv[pxy][xy] = Qaa;	
					}
				}
			}
		}
		sqnorm = 0;
		for(uint i=0;i<PXY;++i)
			sqnorm += utils::sqnorm_2(V0[i],V1[i],XY);
		
		if (timer.elapsed() > Timeout)
			break;
	}
	while(sqnorm > sqeps);
	
	for(uint i=0;i<PXY;++i){
		delete[] V0[i];
		delete[] V1[i];
	}
	delete[] V0;
	delete[] V1;
	for(uint i=0;i<PXY;++i){
		for(uint j=0;j<XY;++j){
			delete[] R[i][j];
			delete[] P[i][j]; 
		}
		delete[] R[i];
		delete[] P[i];
	}
	delete[] P;
	delete[] R;
}
