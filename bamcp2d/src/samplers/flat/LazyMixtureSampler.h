#pragma once

#include "sampler2D.h"
#include "utils/utils.h"

#include "MixtureSamplerFactory.h"
#include "envs/flat/lazySq2Dgrid.h"

class LazyMixtureSampler : public Sampler2D{
	public:
		LazyMixtureSampler(double gamma,uint _NAT,
				int** _ATobserved,uint** _counts,
				uint STARTX, uint STARTY,
				uint STARTXP, uint STARTYP);
		~LazyMixtureSampler();
		
		SIMULATOR2D* updateMDPSample_ind();
		SIMULATOR2D* updateMDPSample();

		uint getAT(uint x, uint y);

		void render(double,double,double);
		protected:
		void updateR();
		void updateMultParams();
		
		double gamma;
		uint NAT;
		LazySqGrid* simulator;
		int** ATobserved;
		int** ATtmp;
		uint** counts;
	
		double* richDirParameters;
		double* poorDirParameters;

		
		//Cache variables
		uint qq;
		uint XGhalf, YGhalf;

		//Gibb sampling variables
		double** multParams; //last sampled multinomial parameters
		double* countsTmp;
		uint R; //last sampled R
		double Rp[NQ];//P(R | multParams)
		
		double** AT;	
};
