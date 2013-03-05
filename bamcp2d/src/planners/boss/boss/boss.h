#pragma once

/*  BOSS Algorithm implementation
 *  Reference:
 *
 *  A Bayesian Sampling Approach to Exploration in Reinforcement Learning
 *  John Asmuth, Lihong Li, Michael L. Littman, Ali Nouri, David Wingate
 *  UAI 2009 
 */


class SIMULATOR2D;
class SamplerFactory2D;
class Sampler2D;
class State2D;
class Interface;

typedef unsigned int uint;

class BOSS
{
public:

    struct PARAMS
    {
        PARAMS();

        int Verbose;
				uint K;
				uint B;
				double epsilon;
				double Timeout;
    };

    BOSS(const SIMULATOR2D& simulator, const PARAMS& params,
				SamplerFactory2D& sampFact,State2D* startState, Interface*
				interface);
    ~BOSS();

    uint SelectAction(State2D* state);
    bool Update(State2D* state, uint action, State2D* observation, double reward);

		//void createMergedModel();
   
	private:
		uint* counts;
    const SIMULATOR2D& Simulator;
		
		//Current mdp sample
		SIMULATOR2D* mdp;
		
		//BOSS-specific
		bool do_sample;
		uint* qcounts;
		PARAMS Params;
		
		//Merged model
		//uint Am;	
		uint** RLPI;
		
		//Cached values
		uint S,A,SA,X,Y;

		SamplerFactory2D& SampFact;

		Sampler2D* MDPSampler;

		//Display-----------
		Interface* display;
		double sumReward;
		uint step;
		void Display(State2D* currentState);
		char dispstr[256];


};

