#pragma once

#include "sampler.h"
#include "utils/utils.h"

#include "envs/mazedoor.h"

class DoorMDPSampler : public Sampler{
	public:
		DoorMDPSampler(const uint* counts, double alpha,
				uint S, uint A, double* R, bool rsas, double gamma); 
		~DoorMDPSampler();

		SIMULATOR* updateMDPSample();
	protected:

		const uint* counts;
		double alpha;
		uint S,A,SA;
		double* R;
		bool rsas;
		double gamma;
		MazeDoor* simulator;
};
