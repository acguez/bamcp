#pragma once

#include "sampler.h"

class SamplerFactory{
	public:

		virtual ~SamplerFactory(){};
	
		virtual Sampler* getTransitionSampler(const uint* counts,uint s, uint a, uint S){return 0;}
		virtual Sampler* getMDPSampler(const uint* counts, uint S, uint A, double* R, bool rsas, double gamma){return 0;}
		virtual Sampler* getTransitionParamSampler(const uint* counts, uint s, uint a, uint S){return 0;}

		virtual void updateCounts(uint s, uint a, uint obs){} //Give incremental info to the sampler for better performance
		virtual void reset(){} //Reset factory

		virtual double getAlphaMean(){return 0;}
};
