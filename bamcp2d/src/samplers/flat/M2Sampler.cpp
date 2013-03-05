#include "M2Sampler.h"
#include "M2SamplerFactory.h"

M2Sampler::M2Sampler(double _gamma, uint _RLocation):
gamma(_gamma), RLocation(_RLocation) {
	simulator = new SqGrid(XM2,YM2,6,gamma);
	simulator->setMoveCost(M2MC);
	simulator->setATr(1,GR);
	simulator->setSTART(SX,SY);
}
M2Sampler::~M2Sampler(){
	if(simulator)
		delete simulator;
}

SIMULATOR2D* M2Sampler::updateMDPSample_ind(){
	return updateMDPSample();	
}

// RLocation (L=0,T=1,R=2,LT=3,LR=4,TR=5,LRT=6)
SIMULATOR2D* M2Sampler::updateMDPSample(){
	//Probably the worst way to write this function
	
	double draw = utils::rng.rand_closed01();
	if(RLocation == 0 ){
		simulator->setATxy(0,XM2RR,YM2R);
		simulator->setATxy(0,XM2TR,YM2TR);
		simulator->setATxy(1,XM2LR,YM2R); //AT=1 => reward by default
		simulator->setATxy(3,XM2S,YM2S);
	}
	else if(RLocation == 1){
		simulator->setATxy(0,XM2RR,YM2R);
		simulator->setATxy(0,XM2LR,YM2R); 
		simulator->setATxy(1,XM2TR,YM2TR);
		simulator->setATxy(4,XM2S,YM2S);
	}
	else if(RLocation == 2){
		simulator->setATxy(1,XM2RR,YM2R);
		simulator->setATxy(0,XM2LR,YM2R); 
		simulator->setATxy(0,XM2TR,YM2TR);
		simulator->setATxy(5,XM2S,YM2S);
	}
	else if(RLocation == 6){
		if(draw < 0.333333){
			simulator->setATxy(0,XM2RR,YM2R);
			simulator->setATxy(0,XM2TR,YM2TR);
			simulator->setATxy(1,XM2LR,YM2R); 
			simulator->setATxy(3,XM2S,YM2S);
		}
		else if(draw < 0.666666){
			simulator->setATxy(0,XM2RR,YM2R);
			simulator->setATxy(1,XM2TR,YM2TR);
			simulator->setATxy(0,XM2LR,YM2R); 
			simulator->setATxy(4,XM2S,YM2S);
		}
		else{
			simulator->setATxy(1,XM2RR,YM2R);
			simulator->setATxy(0,XM2LR,YM2R); 
			simulator->setATxy(0,XM2TR,YM2TR);
			simulator->setATxy(5,XM2S,YM2S);
		}
	}
	else if(RLocation == 3){
		if(draw < 0.5){
			simulator->setATxy(0,XM2RR,YM2R);
			simulator->setATxy(0,XM2TR,YM2TR);
			simulator->setATxy(1,XM2LR,YM2R); 
			simulator->setATxy(3,XM2S,YM2S);
		}
		else{
			simulator->setATxy(0,XM2RR,YM2R);
			simulator->setATxy(1,XM2TR,YM2TR);
			simulator->setATxy(0,XM2LR,YM2R); 
			simulator->setATxy(4,XM2S,YM2S);
		}
	}
	else if(RLocation == 4){
		if(draw < 0.5){
			simulator->setATxy(0,XM2RR,YM2R);
			simulator->setATxy(0,XM2TR,YM2TR);
			simulator->setATxy(1,XM2LR,YM2R); 
			simulator->setATxy(3,XM2S,YM2S);
		}else{
			simulator->setATxy(1,XM2RR,YM2R);
			simulator->setATxy(0,XM2LR,YM2R); 
			simulator->setATxy(0,XM2TR,YM2TR);
			simulator->setATxy(5,XM2S,YM2S);
		}
	}
	else{ //RLocation == 5
			if(draw < 0.5){
			simulator->setATxy(0,XM2RR,YM2R);
			simulator->setATxy(1,XM2TR,YM2TR);
			simulator->setATxy(0,XM2LR,YM2R); 
			simulator->setATxy(4,XM2S,YM2S);
		}else{
			simulator->setATxy(1,XM2RR,YM2R);
			simulator->setATxy(0,XM2LR,YM2R); 
			simulator->setATxy(0,XM2TR,YM2TR);
			simulator->setATxy(5,XM2S,YM2S);
		}
	}

	//std::cout << "L ";

	return (SIMULATOR2D*) simulator;	
}
