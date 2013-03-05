#include "DoorSampler.h"
#include "envs/basicMDP.h"

#include "envs/mazedoor.h"

DoorMDPSampler::DoorMDPSampler(const uint* _counts, double _alpha,
				uint _S, uint _A, double* _R, bool _rsas, double _gamma):
counts(_counts),alpha(_alpha), S(_S), A(_A), R(_R), rsas(_rsas),gamma(_gamma){
	SA = S*A;		
	simulator = new MazeDoor(true,gamma);
}
DoorMDPSampler::~DoorMDPSampler(){
	if(simulator)
		delete simulator;
}

SIMULATOR* DoorMDPSampler::updateMDPSample(){
		
	//Did we observe the door?
	if(counts[4*SA+1*S+6] > 0 || counts[11*SA+3*S+6] > 0)
		simulator->setOpen();
	else if(counts[4*SA+1*S+4] > 0 || counts[11*SA+3*S+11] > 0)
		simulator->setClosed();
	else{
		if(utils::rng.rand_closed01() < alpha)
			simulator->setOpen();
		else
			simulator->setClosed();
	}
	
	return (SIMULATOR*) simulator;	
}
