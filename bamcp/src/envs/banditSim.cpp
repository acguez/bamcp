#include "banditSim.h"
#include "utils/utils.h"

using namespace UTILS;

BANDIT::BANDIT(int _numArms, double* _p, double discount)
:   numArms(_numArms), p(_p)
{
    NumActions = numArms;
    NumObservations = 2;
    RewardRange = 1;
    Discount = discount;
		double pBestArm = 0;
		bestArm = 0;
		for(int i=0;i<numArms;++i){
			if(p[i] > pBestArm){
				bestArm = i;
				pBestArm = p[i];
			}
		}
}

void BANDIT::Validate(const uint& state) const
{}

uint BANDIT::CreateStartState() const
{
		return 0;
}

//Clunky implementation of the bandit
bool BANDIT::Step(uint state, uint action, 
    uint& observation, double& reward) const
{
	
	if(utils::rng.rand_closed01() < p[action]){
		reward = 1;
		observation = 0;
	}	
	else{
		reward = 0;
		observation = 1;
	}
	
	//Never terminates
	return false;
}



void BANDIT::DisplayState(const uint& state, std::ostream& ostr) const
{
}

void BANDIT::DisplayObservation(const uint& state, uint observation, std::ostream& ostr) const
{
}

void BANDIT::DisplayAction(uint action, std::ostream& ostr) const
{
        ostr << action << std::endl;
}
