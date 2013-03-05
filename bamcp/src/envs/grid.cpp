#include "grid.h"
#include "../utils/utils.h"
#include <algorithm>

using namespace UTILS;

Grid::Grid(uint _L, double discount)
:   L(_L)
{
		A = 4;
		S = L*L;
		NumObservations = S;
		NumActions = A;
		Discount = discount;
		rsas = false;

    RewardRange = 1;
			
		Tc = new double[2];
		Tc[1] = 0.8;
		Tc[0] = 0.2;

		//Standard description
		T = 0;

		R = new double[S*A];
		std::fill(R,R+S*A,0);
		R[(S-1)*A] = 1;
		R[(S-1)*A+1] = 1;
		R[(S-1)*A+2] = 1;
		R[(S-1)*A+3] = 1;
}

Grid::~Grid(){
	if(R != 0)
		delete[] R;
	if(T != 0)
		delete[] T;
	delete[] Tc;
}

uint Grid::CreateStartState() const
{ 
		return 0; //(rand() % S);
}


void Grid::Validate(const uint state) const{

}

bool Grid::Step(uint state, uint action, 
    uint& observation, double& reward) const
{
	uint success;
	reward = R[state*A+action];
	utils::rng.multinom(Tc,2,success);
	
	//Action mapping
	//       0
	//       ^
	//       |
	//  3 <-- --> 1
	//       |
	//       v
	//       2
	if(!success){
		if(utils::rng.rand_closed01() < 0.5)
			action = (action + 1) % A;
		else
			action = (action - 1) % A;
	}

	//Decompress state
	uint x = state/L;
	uint y = state-L*x;
	observation = state;
	
	if(action == 0 && y < L-1)
		observation = x*L+y+1; 
	else if(action == 1 && x < L-1)
		observation = (x+1)*L+y; 
	else if(action == 2 && y > 0)
		observation = x*L+y-1;
	else if(action == 3 && x > 0)
		observation = (x-1)*L+y;

	//If at goal
	if(x == L-1 && y == L-1) 
		observation = 0;

//	uint xp = observation/L;
	//uint yp = observation-L*xp;
	//std::cout << x << " " << y << "- " << action << " -> " << xp << " " << yp << " r:" << reward << std::endl; 
	//Never terminates
	return false;
}

void Grid::DisplayState(const uint state, std::ostream& ostr) const
{}

void Grid::DisplayObservation(const uint state, uint observation, std::ostream& ostr) const
{}

void Grid::DisplayAction(int action, std::ostream& ostr) const{
        ostr << action << std::endl;
}
