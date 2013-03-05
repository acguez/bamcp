#pragma once


#include "sampler2D.h"
#include "utils/utils.h"

#include "envs/flat/sq2Dgrid.h"

class M2Sampler : public Sampler2D{
	public:
		M2Sampler(double gamma, uint _RLocation);
		~M2Sampler();
		
		SIMULATOR2D* updateMDPSample_ind();
		SIMULATOR2D* updateMDPSample();
	protected:

		double gamma;
		uint RLocation;
		SqGrid* simulator;
};
