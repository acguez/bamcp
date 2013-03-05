#pragma once 
#include "envs/flat/simulator2D.h"

class Sampler2D{
	
	public:
		virtual ~Sampler2D(){};
		
		//virtual uint getNextStateSample(){return 0;};
		
		virtual SIMULATOR2D* updateMDPSample_ind(){return 0;}; //Get independent samples
		virtual SIMULATOR2D* updateMDPSample(){return 0;}; 
		//virtual void getNextTParamSample(double* P){};
};
