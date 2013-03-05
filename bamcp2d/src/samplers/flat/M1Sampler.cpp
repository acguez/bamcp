#include "M1Sampler.h"
#include "M1SamplerFactory.h"

M1Sampler::M1Sampler(double _gamma, bool _sensorObserved, bool _leftR):
gamma(_gamma), sensorObserved(_sensorObserved), leftR(_leftR) {
	simulator = new SqGrid(XM1,YM1,5,gamma);
	simulator->setMoveCost(MC);
	simulator->setATr(1,GR);
	simulator->setSTART(SX,SY);
}
M1Sampler::~M1Sampler(){
	if(simulator)
		delete simulator;
}

SIMULATOR2D* M1Sampler::updateMDPSample_ind(){
	return updateMDPSample();
}
SIMULATOR2D* M1Sampler::updateMDPSample(){
	if((sensorObserved && leftR) || (!sensorObserved && utils::rng.rand_closed01() < 0.5)){
		simulator->setATxy(0,XM1RR,YM1R);
		simulator->setATxy(1,XM1LR,YM1R); //AT=1 => reward by default
		simulator->setATxy(3,XM1S,YM1S);
		//std::cout << "R ";
	}
	else{
		simulator->setATxy(1,XM1RR,YM1R);
		simulator->setATxy(0,XM1LR,YM1R); //AT=1 => reward by default
		simulator->setATxy(4,XM1S,YM1S);

		//std::cout << "L ";
	}

	return (SIMULATOR2D*) simulator;	
}
