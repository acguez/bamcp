#pragma once

#include "samplerFactory2D.h"

//Maze size
#define XG 20
#define YG 20
//Number of quadrants
#define NQ 4

#define PREDCOSTCONST 2
#define PREDWAITCONST 2


class MixtureSamplerFactory : public SamplerFactory2D{
	public:
		
		MixtureSamplerFactory(bool lazy,int mode);
	
		~MixtureSamplerFactory();

		Sampler2D* getMDPSampler(double gamma);
		
		void reset();

		void firstObs(State2D* state);
		void update(State2D* state, uint a,  State2D* obs);
		
		virtual void render(double startx, double starty, double sqsz);

		virtual void Clone(SamplerFactory2D* parent);
		virtual SamplerFactory2D* createNewClone();

	protected:
		void extractObs(State2D* state);
		int** ATobserved; //Observation book keeping
		uint** counts;  //Observation summary

		uint STARTX,STARTY;
		uint STARTXP,STARTYP;
		//NumAT
		uint NAT;		
		bool lazy;
		int mode;
		//For rendering
		double** ATcolor;
};
