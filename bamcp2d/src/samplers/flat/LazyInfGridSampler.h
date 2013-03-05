#pragma once

#include "sampler2D.h"
#include "utils/utils.h"

#include "InfGridSamplerFactory.h"
class LazyInfGrid;


class LazyInfGridSampler : public Sampler2D{
	public:
		LazyInfGridSampler(double gamma,
				int** _ATobserved,
				uint** _counts_col,
				uint** _counts_row,
				std::vector<Pair> *PairObserved,
				uint _boundX, uint _boundY,
				uint STARTX, uint STARTY,
				double* _beta_params_col,
				double* _beta_params_row,
				double* _pcol,
				double* _prow);

		~LazyInfGridSampler();
		
		SIMULATOR2D* updateMDPSample_ind();
		SIMULATOR2D* updateMDPSample();

		uint getAT(uint x, uint y);
		
		void setLastPos(uint x, uint y);
		void setLastPMeans(double* _pcol_mean,double* _prow_mean,
				uint* _pcol_upcounts, uint* _prow_upcounts,uint lasta);
		void render(double,double,double);
		protected:
	
		double gamma;
	
		LazyInfGrid* simulator;
	
		int** ATobserved;

		// -1 -> not yet sampled (or if ATobserved != -1, was observed directly => reward absent)
		// 0 -> reward consumed during simulation 
		int** ATmap;
		//ATmap scheme (to avoid resetting at every update):
		// < updatesAT -> not yet sampled (or if ATobserved != -1, was observed directly => reward absent)
		// = updatesAT -> reward consumed during simulation 

		uint** counts_col;
		uint** counts_row;
		
		std::vector<Pair> *PairObserved;
		
		uint boundX;
		uint boundY;
		
		double* beta_params_col;
		double* beta_params_row;
		double colmeanf;
		double rowmeanf;
		
		//For MCMC sampler
		void updateP();
		//double* pcol_prop;
		//double* prow_prop;
		double pcol_propxx;
		double prow_propyy;
		
		double* pcol;
		double* pcol_un; // Sampled from prior
		uint* pcol_age;

		double* prow;
		double* prow_un; // Sampled from prior
		uint* prow_age;
		
	//	uint* pcol_dirty;
	//	uint* prow_dirty;
		uint dirtyCount;

		double* param;
		double* beta_params;

		uint accepts;
		uint updates;
		int updatesAT;
		
		//For rendering
		uint lastx;
		uint lasty;

#if defined(GX)
		std::vector<Pair> SRObserved; // Simulated reward locations
		uint* pcol_upcounts;
		uint* prow_upcounts;
		double* pcol_mean;
		double* prow_mean;
#endif
};
