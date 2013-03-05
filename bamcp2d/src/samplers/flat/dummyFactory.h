#pragma once 

#include "samplerFactory2D.h"
#include "sampler2D.h"
#include "envs/flat/simulator2D.h"

/*  Dummy sampler factory that always returns an environment
 *  given as argument.
 */

class DummyFactory : public SamplerFactory2D{
	public:
		DummyFactory(SIMULATOR2D* _real);
		Sampler2D* getMDPSampler(double gamma);
		
		void reset(){}
		void update(State2D* state, uint a,  State2D* obs){}
		
		virtual void render(double startx, double starty, double sqsz);
		virtual void Clone(SamplerFactory2D* parent);
		virtual SamplerFactory2D* createNewClone();
	protected:
		SIMULATOR2D* real;
};


class DummySampler : public Sampler2D{
	public:
		DummySampler(SIMULATOR2D* _real): real(_real){}
		SIMULATOR2D* updateMDPSample_ind(){return real;}
		SIMULATOR2D* updateMDPSample(){return real;}
	protected:
		SIMULATOR2D* real;
};
