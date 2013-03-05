#include "experiment.h"
#include "boost/timer.hpp"

using namespace std;

EXPERIMENT::PARAMS::PARAMS()
:   NumRuns(100),
    NumSteps(300),
    SimSteps(1000),
    TimeOut(15000),
    Accuracy(0.01),
    AutoExploration(true),
    UndiscountedHorizon(1000)
{
}

EXPERIMENT::EXPERIMENT(SIMULATOR2D& real,
    SIMULATOR2D& simulator, const string& outputFile,
    EXPERIMENT::PARAMS& expParams, BAMCP2D::PARAMS& searchParams, SamplerFactory2D& _samplerFact,
		Interface* _display)
:   Real(real),
    Simulator(simulator),
    ExpParams(expParams),
    SearchParams(searchParams),
		samplerFact(_samplerFact),
    OutputFile(outputFile.c_str()),
		display(_display)
{
    if (ExpParams.AutoExploration)
    {
            SearchParams.ExplorationConstant = simulator.GetRewardRange();
    }

    BAMCP2D::InitFastUCB(SearchParams.ExplorationConstant);
}

void EXPERIMENT::Run(std::vector<double>& Rhist, std::vector<uint>& Ahist)
{
		Real.reset();

    boost::timer timer;
   
		State2D* state = new State2D(Real.GetNumC());
		State2D* tmp;
		State2D* observation = new State2D(Real.GetNumC());
		
		Real.CreateStartState(state);
		Real.CreateStartState(observation);
		
		if(display){ //Draw twice to get initial frame (b/c of double buffering)
			display->drawAgent(state->x,state->y);
			display->StopRender();
			display->StartRender();
			display->drawAgent(state->x,state->y);
		}
    BAMCP2D mcts(Simulator, SearchParams, samplerFact, state, display);
	

    double undiscountedReturn = 0.0;
    double discountedReturn = 0.0;
    double discount = 1.0;
    bool terminal = false;
    int t;

    //if (SearchParams.Verbose >= 1)
        //Real.DisplayState(state, cout);

    for (t = 0; t < ExpParams.NumSteps; t++)
    {
        double reward;
        uint action = mcts.SelectAction(state);

        terminal = Real.Step(state, action, observation, reward);
				Rhist.push_back(reward);
				Ahist.push_back(action);
        
				//std::cout << state << " " << action << " " << observation << " " << reward << std::endl;
				
				Results.Reward.Add(reward);
        undiscountedReturn += reward;
        discountedReturn += reward * discount;
        discount *= Real.GetDiscount();

        if (terminal)
        {
            cout << "Terminated" << endl;
            break;
        }
        mcts.Update(state, action, observation, reward);
				
				tmp = state;
				//For MDP:
				state = observation;
				observation = tmp;
				
			
				if (timer.elapsed() > ExpParams.TimeOut)
        {
            cout << "Timed out after " << t << " steps in "
                << Results.Time.GetTotal() << "seconds" << endl;
            break;
        }
    }

    Results.Time.Add(timer.elapsed());
    Results.UndiscountedReturn.Add(undiscountedReturn);
    Results.DiscountedReturn.Add(discountedReturn);
    cout << "(" << discountedReturn << "," 
			<< Results.DiscountedReturn.GetMean() << ":"
			<< undiscountedReturn << "," << Results.UndiscountedReturn.GetMean()
			<< ") " << flush;

		delete state;
		delete observation;
}

