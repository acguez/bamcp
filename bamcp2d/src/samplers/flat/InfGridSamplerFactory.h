#pragma once

#define CHUNKX 1000
#define CHUNKY 1000


#include "samplerFactory2D.h"

#include <vector>
typedef std::pair<uint,uint> Pair;

class InfGridSamplerFactory : public SamplerFactory2D{
	public:
		
		InfGridSamplerFactory(double* _beta_params_col,
		double* _beta_params_row);
	
		~InfGridSamplerFactory();

		Sampler2D* getMDPSampler(double gamma);
		
		void reset();

		void firstObs(State2D* state);
		void update(State2D* state, uint a,  State2D* obs);
		
		virtual void render(double startx, double starty, double sqsz);

		virtual void Clone(SamplerFactory2D* parent);
		virtual SamplerFactory2D* createNewClone();

	protected:
	
		void extractObs(State2D* state);
		double* beta_params_col;
		double* beta_params_row;

		int** ATobserved; //Observation book keeping 
		//  -1   Not observed
		//  0    Observed -> no reward
		//  1    Observed -> reward ( => reward no longer there)
	
		//Summary statistics
		std::vector<Pair> *PairObserved; //Which pairs have been observed

		uint** counts_col;
		uint** counts_row;
		
		double* pcol;
		double* prow;

		uint STARTX,STARTY;
		
		uint boundX;
		uint boundY;
		//NumAT
		uint NAT;		

		//For rendering
		double** ATcolor;
		
		//Last state position observed for rendering agent centered observations
		uint lastx;
		uint lasty;
		uint lasta;		
		
#if defined(GX)
		double* pcol_mean;
		double* prow_mean;
		uint* pcol_upcounts;
		uint* prow_upcounts;
#endif

};
