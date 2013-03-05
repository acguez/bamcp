#include "experiment_boss.h"
#include "boost/timer.hpp"

using namespace std;

EXPERIMENT_BOSS::PARAMS::PARAMS()
:   NumRuns(100),
    NumSteps(300),
    SimSteps(1000),
    TimeOut(15000),
    MinDoubles(0),
    MaxDoubles(20),
    TransformDoubles(-4),
    TransformAttempts(1000),
    Accuracy(0.01),
    UndiscountedHorizon(1000),
    AutoExploration(true)
{
}

EXPERIMENT_BOSS::EXPERIMENT_BOSS(SIMULATOR2D& real,
    SIMULATOR2D& simulator, const string& outputFile,
    EXPERIMENT_BOSS::PARAMS& expParams, BOSS::PARAMS& searchParams, SamplerFactory2D& _samplerFact,
		Interface* _display)
:   Real(real),
    Simulator(simulator),
    ExpParams(expParams),
    SearchParams(searchParams),
		samplerFact(_samplerFact),
    OutputFile(outputFile.c_str()),
		display(_display)
{
    

}

void EXPERIMENT_BOSS::Run(std::vector<double>& Rhist)
{
    boost::timer timer;
		
		Real.reset();

		State2D* state = new State2D(Real.GetNumC());
		State2D* tmp;
		State2D* observation = new State2D(Real.GetNumC());
		
		Real.CreateStartState(state);
		
		if(display){ //Draw twice to get initial frame (b/c of double buffering)
			display->drawAgent(state->x,state->y);
			display->StopRender();
			display->StartRender();
			display->drawAgent(state->x,state->y);
		}
    BOSS boss(Simulator, SearchParams, samplerFact, state, display);
	

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
        uint action = boss.SelectAction(state);

        terminal = Real.Step(state, action, observation, reward);
				Rhist.push_back(reward);
        
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
        boss.Update(state, action, observation, reward);
				
				tmp = state;
				
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
		//cout << "Discounted return = " << discountedReturn
    //    << ", average = " << Results.DiscountedReturn.GetMean() << endl;
    //cout << "Undiscounted return = " << undiscountedReturn
    //    << ", average = " << Results.UndiscountedReturn.GetMean() << endl;
		//

		delete state;
		delete observation;
}

//----------------------------------------------------------------------------
