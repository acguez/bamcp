#pragma once

#include "sampler2D.h"
#include "utils/utils.h"

#include "MixtureSamplerFactory.h"
#include "envs/flat/sq2Dgrid.h"

class MixtureSampler : public Sampler2D{
	public:
		MixtureSampler(double gamma,uint _NAT,
				int** _ATobserved,uint** _counts,
				uint STARTX, uint STARTY,
			uint STARTXP, uint STARTYP);
		~MixtureSampler();

		SIMULATOR2D* updateMDPSample_ind();
		SIMULATOR2D* updateMDPSample();
		protected:
		void updateR();
		void updateMultParams();
		
		double gamma;
		uint NAT;
		SqGrid* simulator;
		int** ATobserved;
		uint** counts;
	
		double* richDirParameters;
		double* poorDirParameters;


		//Gibb sampling variables
		double** multParams; //last sampled multinomial parameters
		double* countsTmp;
		uint R; //last sampled R
		double Rp[NQ];//P(R | multParams)
		
		double** AT;	
};
