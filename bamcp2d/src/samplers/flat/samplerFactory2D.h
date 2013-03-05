#pragma once

#include "sampler2D.h"

#include "envs/flat/simulator2D.h"


class SamplerFactory2D{
	public:

		virtual ~SamplerFactory2D(){};
	
		//virtual Sampler* getTransitionSampler(State2D* s, uint a){return 0;}
		virtual Sampler2D* getMDPSampler(double gamma){return 0;}
		//virtual Sampler* getTransitionParamSampler(){return 0;}
		
		virtual void firstObs(State2D* state){}

		virtual void Clone(SamplerFactory2D* parent){}
		virtual SamplerFactory2D* createNewClone(){return 0;}

		virtual void update(State2D* state, uint a, State2D* observation){} //Update posterior (observation = next state)

		virtual void reset(){} //Reset factory

		virtual void render(double startx, double starty, double sqsz){};
};
