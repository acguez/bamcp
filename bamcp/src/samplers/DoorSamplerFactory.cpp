#include "DoorSamplerFactory.h"
#include "FDMTransitionSampler.h"
#include "DoorSampler.h"

DoorsamplerFactory::DoorsamplerFactory(double prob) : alpha(prob){ }

//NOT IMPLEMENTED FOR ALL SAMPLERS
//Default behavior for non-MDP samplers
Sampler* DoorsamplerFactory::getTransitionSampler(const uint* counts,uint s, uint a, uint S){
	double* P = new double[S];
	double sum = 0;
	for(uint ll=0;ll<S;++ll){
		P[ll] = (double) counts[ll] + alpha;	
		sum += P[ll];
	}
	//Normalize 
	for(uint ll=0;ll<S;++ll){
		P[ll] = P[ll] / sum; 
	}
	
	FDMTransitionSampler* sampler = new FDMTransitionSampler(P,S);
	return (Sampler*) sampler;
}

Sampler* DoorsamplerFactory::getTransitionParamSampler(const uint* counts, uint s, uint a, uint S){
	FDMTransitionParamSampler* sampler = new FDMTransitionParamSampler(counts,alpha,S);
	return (Sampler*) sampler;
}

//ONLY ONE IMPLEMENTED 
Sampler* DoorsamplerFactory::getMDPSampler(const uint* counts, uint S, uint A,
		double* R, bool rsas, double gamma){
	DoorMDPSampler* sampler = new DoorMDPSampler(counts, alpha, S, A, R, rsas, gamma);
	return (Sampler*) sampler;
}


