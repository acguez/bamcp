#include "mazedoor.h"
#include "../utils/utils.h"
#include <algorithm>

using namespace UTILS;

MazeDoor::MazeDoor(bool _open, double discount)
{
		open = _open;
		A = 4;
		S = 17;
		SA = S*A;	
		NumObservations = S;
		NumActions = A;
		Discount = discount;
		rsas = false;

    RewardRange = 1;
			
		Tc = new double[2];
		Tc[1] = 0.9;
		Tc[0] = 0.1;
	
	//Action mapping
	//       0
	//       ^
	//       |
	//  3 <-- --> 1
	//       |
	//       v
	//       2
	
		/*  Worst grid configuration ever... 
		 *  0 is the start state
		 *
		 *   2 5 7  12
		 *   1 + 8  13
		 *-->0 + 9  14
		 *   3 + 10 15
		 *   4 6 11 16 <-- Goal
		 */
		//neighbors graph
		int ne[17][4] = {
			// UP RIGHT DOWN LEFT
			{1, 0, 3, 0}, //0
			{2, 1, 0, 1}, //1
			{2, 5, 1, 2}, //2
			{0, 3, 4, 3}, //3
			{3, 6, 4, 4}, //4
			{5, 7, 5, 2}, //5
			{6, 11, 6, 4}, //6
			{7, 12, 8, 5}, //7
			{7, 13, 9, 8}, //8
			{8, 14, 10, 9}, //9
			{9, 15, 11, 10}, //10
			{10, 16, 11, 6}, //11
			{12, 12, 13, 7}, //12
			{12, 13, 14, 8}, //13
			{13, 14, 15, 9}, //14
			{14, 15, 16, 10}, //15
		  {0, 0, 0, 0} //16
		};
		T = new double[SA*S];
		std::fill(T,T+SA*S,0);
		
		if(!open){
			ne[4][1] = 4;
			ne[11][3] = 11;
		}

		//First write generic grid transitions
		for(uint s=0;s<S;++s){
			//Action 0
			T[s*SA+ne[s][0]] += Tc[1];
			T[s*SA+ne[s][3]] += Tc[0]/2;
			T[s*SA+ne[s][1]] += Tc[0]/2;
			//Action 2
			T[s*SA+2*S+ne[s][2]] += Tc[1];
			T[s*SA+2*S+ne[s][3]] += Tc[0]/2;
			T[s*SA+2*S+ne[s][1]] += Tc[0]/2;
			//Action 3
			T[s*SA+3*S+ne[s][3]] += Tc[1];
			T[s*SA+3*S+ne[s][0]] += Tc[0]/2;
			T[s*SA+3*S+ne[s][2]] += Tc[0]/2;
			//Action 1
			T[s*SA+1*S+ne[s][1]] += Tc[1];
			T[s*SA+1*S+ne[s][0]] += Tc[0]/2;
			T[s*SA+1*S+ne[s][2]] += Tc[0]/2;
		}

		R = new double[S*A];
		std::fill(R,R+S*A,0);
		R[(S-1)*A] = 1;
		R[(S-1)*A+1] = 1;
		R[(S-1)*A+2] = 1;
		R[(S-1)*A+3] = 1;

}


MazeDoor::~MazeDoor(){
	if(R != 0)
		delete[] R;
	if(T != 0)
		delete[] T;
	delete[] Tc;
}


void MazeDoor::setClosed(){
	if(open){
		T[4*SA+1*S+6] = 0;
		T[4*SA+0*S+6] = 0;
		T[4*SA+2*S+6] = 0;
		T[4*SA+1*S+4] += Tc[1];
		T[4*SA+2*S+4] += Tc[0]/2;
		T[4*SA+0*S+4] += Tc[0]/2;

		T[11*SA+3*S+6] = 0;
		T[11*SA+0*S+6] = 0;
		T[11*SA+2*S+6] = 0;
		T[11*SA+3*S+11] += Tc[1];
		T[11*SA+2*S+11] += Tc[0]/2;
		T[11*SA+0*S+11] += Tc[0]/2;

		open = false;
	}
}
void MazeDoor::setOpen(){
	if(!open){
		T[4*SA+1*S+6] = Tc[1];
		T[4*SA+0*S+6] = Tc[0]/2;
		T[4*SA+2*S+6] = Tc[0]/2;	
		T[4*SA+1*S+4] -= Tc[1];
		T[4*SA+2*S+4] -= Tc[0]/2;
		T[4*SA+0*S+4] -= Tc[0]/2;

		T[11*SA+3*S+6] = Tc[1];
		T[11*SA+0*S+6] = Tc[0]/2;
		T[11*SA+2*S+6] = Tc[0]/2;
		T[11*SA+3*S+11] -= Tc[1];
		T[11*SA+2*S+11] -= Tc[0]/2;
		T[11*SA+0*S+11] -= Tc[0]/2;
		open = true;
	}
}

uint MazeDoor::CreateStartState() const
{ 
		return 0; //(rand() % S);
}


void MazeDoor::Validate(const uint state) const{} 

bool MazeDoor::Step(uint state, uint action, 
    uint& observation, double& reward) const
{
	utils::rng.multinom(T+state*SA+action*S,S,observation);
	reward = R[state*A+action];
	
	//Never terminates
	return false;
}

void MazeDoor::DisplayState(const uint state, std::ostream& ostr) const {}

void MazeDoor::DisplayObservation(const uint state, uint observation, std::ostream& ostr) const {}

void MazeDoor::DisplayAction(int action, std::ostream& ostr) const {
        ostr << action << std::endl;
}
