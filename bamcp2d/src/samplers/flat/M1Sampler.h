#pragma once

#include "sampler2D.h"
#include "utils/utils.h"

#include "envs/flat/sq2Dgrid.h"

class M1Sampler : public Sampler2D{
	public:
		M1Sampler(double gamma, bool sensorObserved, bool leftR);
		~M1Sampler();


		SIMULATOR2D* updateMDPSample_ind();
		SIMULATOR2D* updateMDPSample();
	protected:

		double gamma;
		bool sensorObserved, leftR;
		SqGrid* simulator;
};
