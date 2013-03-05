#pragma once

#include "samplerFactory2D.h"

//Maze size
#define XM2 19
#define YM2 12
//Rewards location
#define XM2LR 0
#define XM2RR 18
#define YM2R 2
#define XM2TR 9
#define YM2TR 11
//Sensor location
#define XM2S 9
#define YM2S 0

#define GR 1.0
#define M2MC 0.05
#define SX 9
#define SY 2

class M2SamplerFactory : public SamplerFactory2D{
	public:
		
		M2SamplerFactory();
	
		Sampler2D* getMDPSampler(double gamma);
		
		void reset(){RLocation = 6;}

		virtual void Clone(SamplerFactory2D* parent);
		virtual SamplerFactory2D* createNewClone();

		void update(State2D* state, uint a,  State2D* obs);

	protected:
		
		uint RLocation; //Reward location (L=0,T=1,R=2,LT=3,LR=4,TR=5,LRT=6) 
};
