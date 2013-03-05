#include "doubleloop.h"
#include "../utils/utils.h"
#include <algorithm>

using namespace UTILS;

Dloop::Dloop(double discount) 
{
	  S = 9;
		A = 2;
		NumObservations = S;
		NumActions = 2;
		Discount = discount;
		rsas = false;

    RewardRange = 2;

		//Standard description
		T = 0;

		R = new double[S*A];
		std::fill(R,R+S*A,0);
		R[8*A+0] = 2; //1;
		R[8*A+1] = 2; //1; 
		R[4*A+0] = 1; // 0.5;
		R[4*A+1] = 1; // 0.5;
}

Dloop::~Dloop(){
	if(R != 0)
		delete[] R;
	if(T != 0)
		delete[] T;
}

uint Dloop::CreateStartState() const
{ 
		return 0; //(rand() % S);
}


void Dloop::Validate(const uint state) const{

}

bool Dloop::Step(uint state, uint action, 
    uint& observation, double& reward) const
{
	reward = R[state*A+action];
	if(state == 0)
		if(action == 0)
			observation = 1;
		else 
			observation = 5;
	else if(state == 8 || state == 4)
		observation = 0;
	else if(state < 4)
		observation = state + 1;
	else{ //left side of loop
		if(action == 0)
			observation = 0; //reset
		else
			observation = state + 1;
	}

	//std::cout << state << " " << action << " " << observation << " " << reward << std::endl;
	//Never terminates
	return false;
}


void Dloop::DisplayState(const uint state, std::ostream& ostr) const
{
    
}

void Dloop::DisplayObservation(const uint state, uint observation, std::ostream& ostr) const
{
   
}

void Dloop::DisplayAction(int action, std::ostream& ostr) const
{
        ostr << action << std::endl;
}
