#pragma once

/* BFS3 Algorithm implementation
 *
 * Reference:
 * Learning is planning: near Bayes-optimal reinforcement learning via Monte-Carlo tree search  
 * John Asmuth, Michael Littman
 * UAI 2011
 *
 */


class SIMULATOR2D;
class SamplerFactory2D;
class Sampler2D;
class VNODE3;
class QNODE3;
class State2D;
class Interface;

typedef unsigned int uint;

class BFS3
{
public:

    struct PARAMS
    {
        PARAMS();

        int Verbose;
				uint D;
				uint C;
				uint N;
				double Timeout;
				double gamma;
				double Vmin;
				double Vmax;
    };

    BFS3(const SIMULATOR2D& simulator, const PARAMS& params,
				SamplerFactory2D& sampFact, State2D* startState, Interface* interface);
    ~BFS3();
		
    uint SelectAction(State2D* current_state);
    bool Update(State2D* state, uint action, State2D* observation, double reward);
		void bellmanBackup(VNODE3* vnode, int action, State2D* s);



private:

		double FSSS(State2D* prev_state,uint a, State2D* start_state);
		void FSSSRollout(VNODE3*& vnode, State2D* state, uint depth);
		VNODE3* ExpandNode(State2D* state); 
	
		//For display	
		void Display(State2D* currentState);
		uint step;
		double sumRewards;
		char dispstr[256];
		//----------
		
		uint A;

    PARAMS Params;
    VNODE3* Root;
	
		const SIMULATOR2D& Simulator;
	
		SamplerFactory2D& SampFact;
	
		SamplerFactory2D* SampFact_IMG;
		Sampler2D* MDPSampler;
			
		Interface* display;
		
		State2D* state2;
		State2D* observation;
		double reward;

    int TreeDepth, PeakTreeDepth;
};

