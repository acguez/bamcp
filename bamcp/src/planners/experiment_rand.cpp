#include "experiment_rand.h"
#include "boost/timer.hpp"

using namespace std;

EXPERIMENT_RAND::PARAMS::PARAMS()
:   NumRuns(100),
    NumSteps(300),
    SimSteps(1000),
    TimeOut(3600),
    Accuracy(0.01),
    UndiscountedHorizon(1000),
    AutoExploration(true)
{
}

EXPERIMENT_RAND::EXPERIMENT_RAND(const SIMULATOR& real,
    const SIMULATOR& simulator, const string& outputFile,
    EXPERIMENT_RAND::PARAMS& expParams)
:   Real(real),
    Simulator(simulator),
    ExpParams(expParams),
    OutputFile(outputFile.c_str())
{

}

void EXPERIMENT_RAND::Run(std::vector<double>& Rhist)
{
    boost::timer timer;

    double undiscountedReturn = 0.0;
    double discountedReturn = 0.0;
    double discount = 1.0;
    bool terminal = false;
    int t;

		uint A = Simulator.GetNumActions();

    uint state = Real.CreateStartState();

    for (t = 0; t < ExpParams.NumSteps; t++)
    {
        uint observation;
        double reward;
        uint action = rand() % A;
        terminal = Real.Step(state, action, observation, reward);
				Rhist.push_back(reward);
        
				Results.Reward.Add(reward);
        undiscountedReturn += reward;
        discountedReturn += reward * discount;
        discount *= Real.GetDiscount();

				if (terminal)
        {
            cout << "Terminated" << endl;
            break;
        }
					
				//For MDP:
				state = observation;


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
			<< ") ";
}

