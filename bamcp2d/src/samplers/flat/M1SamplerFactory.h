#pragma once

#include "samplerFactory2D.h"

//Maze size
#define XM1 19
#define YM1 10
//Rewards location
#define XM1LR 0
#define XM1RR 18
#define YM1R 2
//Sensor location
#define XM1S 9
#define YM1S 0

#define GR 1.0
#define MC 0.001
#define SX 9
#define SY 2

class M1SamplerFactory : public SamplerFactory2D{
	public:
		
		M1SamplerFactory();
	
		Sampler2D* getMDPSampler(double gamma);
		
		virtual void Clone(SamplerFactory2D* parent);
		virtual SamplerFactory2D* createNewClone();
		void reset(){ObservedRLocation = false;}
		void update(State2D* state, uint a,  State2D* obs);

	protected:
		
		bool ObservedRLocation; // Sensor observed?
		bool LeftR; //Reward is to the left? (only valid if ObservedRLocation==true)
};
