#ifndef BAMCP2D_H
#define BAMCP2D_H

/* BAMCP algorithm implementation (for 2D factored environments)
 *
 * Reference:
 *
 * Guez, A. and Silver, D. and Dayan, P.
 * Efficient Bayes-Adaptive Reinforcement Learning using Sample-Based Search
 * Advances in Neural Information Processing Systems (NIPS) 2012
 *
 */


#include "envs/flat/simulator2D.h"
#include "planners/statistic.h"

#include <fstream>

#define MAXPREDWAIT 5
		
class VNODE2D;
class QNODE2D;
class QNODE2DAT;
class SamplerFactory2D;
class Interface;

class BAMCP2D
{
public:

    struct PARAMS
    {
        PARAMS();

        int Verbose;
        int MaxDepth;
        int NumSimulations;
        int MaxAttempts;
        int ExpandCount;
        double ExplorationConstant;
				int DisplayMode;
				int RolloutMode;
				double Timeout;
				bool ReuseTree;
				double QLepsilon;
				double QLalpha;

    };

    BAMCP2D(SIMULATOR2D& simulator, const PARAMS& params,SamplerFactory2D& sampFact,
				State2D* startState, Interface* interface);
    ~BAMCP2D();

    int SelectAction(State2D* current_state);
    bool Update(State2D* state, uint action, State2D* observation, double reward);

    void UCTSearch(State2D* current_state);

    double Rollout(SIMULATOR2D* mdp, State2D* state);

    void ClearStatistics();
    void DisplayStatistics(std::ostream& ostr) const;

	
		//Viz tree display --------------------------------------------------------
		void Display(State2D* currentState, uint simLeft);
		void DisplayTreeV(VNODE2D* vnode,uint);
		void DisplayTreeQ(QNODE2D& qnode, VNODE2D* parent,uint);
		void DisplayTreeQAT(QNODE2DAT* qnode, VNODE2D* parent,uint);
		//END Viz tree display --------------------------------------------------------
    
		static void InitFastUCB(double exploration);

private:
		//Cached values
		uint A,NumAT,NumC;

    PARAMS Params;
    const SIMULATOR2D& Simulator;
    VNODE2D* Root;
    
		int TreeDepth, PeakTreeDepth;
		
		SamplerFactory2D& SampFact;
		
		//Current mdp sample
		SIMULATOR2D* mdp;
	
		//Tmp value
		State2D* state2;

		//Display-----------
		Interface* display;
		uint** rolloutCountDisp;
		uint maxRolloutVisits;
		std::vector<std::pair<uint,uint> > rolloutHistory;
		uint** drawn;
		double moffsetx, moffsety;

		//Leaf computation
		uint X,Y;
		double*** Q;
		uint** QU;
		bool** XYvisited;
		double* QP;

    STATISTIC StatTreeDepth;
    STATISTIC StatRolloutDepth;
    STATISTIC StatTotalReward;

    int GreedyUCB(VNODE2D* vnode, bool ucb) const;
    int SelectRandom() const;
    double SimulateV(SIMULATOR2D* senv, State2D* state, VNODE2D* vnode);
    double SimulateQ(SIMULATOR2D* senv, State2D* state, QNODE2D& qnode, uint action);
    VNODE2D* ExpandNode(State2D* state); 

    // Fast lookup table for UCB
    static const int UCB_N = 10000, UCB_n = 100;
    static double UCB[UCB_N][UCB_n];
    static bool InitialisedFastUCB;

    double FastUCB(int N, int n, double logN) const;

		uint step;
		double sumRewards;

		char dispstr[256];

		std::ofstream meand;
		std::ofstream maxd;
	
};

#endif // BAMCP2D_H
