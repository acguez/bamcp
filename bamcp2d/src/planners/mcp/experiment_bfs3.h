#pragma once

#include "bfs3/bfs3.h"
#include "envs/flat/simulator2D.h"
#include "samplers/flat/samplerFactory2D.h"

#include "planners/statistic.h"
#include <fstream>
#include "utils/utils.h"

#include "viz/interface.h"
#include "experiment.h"



//----------------------------------------------------------------------------

class EXPERIMENT_BFS3
{
public:

    struct PARAMS
    {
        PARAMS();
        
        int NumRuns;
        int NumSteps;
        int SimSteps;
        double TimeOut;
        double Accuracy;
    };

    EXPERIMENT_BFS3(SIMULATOR2D& real, SIMULATOR2D& simulator, 
        const std::string& outputFile, 
        EXPERIMENT_BFS3::PARAMS& expParams, BFS3::PARAMS& searchParams,
				 SamplerFactory2D& samplerFactory,
				 Interface* _display);

    void Run(std::vector<double>& Rhist);

private:

    SIMULATOR2D& Real;
    SIMULATOR2D& Simulator;
    EXPERIMENT_BFS3::PARAMS& ExpParams;
    BFS3::PARAMS& SearchParams;
    RESULTS Results;
		SamplerFactory2D& samplerFact;
    std::ofstream OutputFile;
		Interface* display;
};

//----------------------------------------------------------------------------

