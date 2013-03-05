#pragma once

#include "envs/flat/simulator2D.h"

#include "planners/statistic.h"
#include <fstream>
#include "utils/utils.h"

#include "planners/mcp/experiment.h"

#define MAXPREDWAIT 5
//----------------------------------------------------------------------------

class EXPERIMENT_BASELINES
{
public:

    struct PARAMS
    {
        PARAMS();
        int NumSteps;
        double TimeOut;
				uint Mode; //Mode:   0 - RAND, 1 - FOVI, 2 - QLearning
				double epsilon; // eps-greedy parameter
				double alpha; //QLearning learning rate
    };

    EXPERIMENT_BASELINES(SIMULATOR2D& real,
				EXPERIMENT_BASELINES::PARAMS& expParams);

		~EXPERIMENT_BASELINES();

    void Run(std::vector<double>& Rhist);

private:
		uint** RLPI;
		uint X,Y;
		double*** Q;
    SIMULATOR2D& Real;
    EXPERIMENT_BASELINES::PARAMS& ExpParams;
};

//----------------------------------------------------------------------------

