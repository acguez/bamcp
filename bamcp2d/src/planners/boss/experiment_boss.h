#pragma once

#include "boss/boss.h"
#include "envs/flat/simulator2D.h"
#include "samplers/flat/samplerFactory2D.h"

#include "planners/statistic.h"
#include <fstream>
#include "utils/utils.h"

#include "viz/interface.h"

#include "planners/mcp/experiment.h"


//----------------------------------------------------------------------------

class EXPERIMENT_BOSS
{
public:

    struct PARAMS
    {
        PARAMS();
        
        int NumRuns;
        int NumSteps;
        int SimSteps;
        double TimeOut;
        int MinDoubles, MaxDoubles;
        int TransformDoubles;
        int TransformAttempts;
        double Accuracy;
        int UndiscountedHorizon;
        bool AutoExploration;
    };

    EXPERIMENT_BOSS(SIMULATOR2D& real, SIMULATOR2D& simulator, 
        const std::string& outputFile, 
        EXPERIMENT_BOSS::PARAMS& expParams, BOSS::PARAMS& searchParams,
				 SamplerFactory2D& samplerFactory,
				 Interface* _display);

    void Run(std::vector<double>& Rhist);

private:

    SIMULATOR2D& Real;
    SIMULATOR2D& Simulator;
    EXPERIMENT_BOSS::PARAMS& ExpParams;
    BOSS::PARAMS& SearchParams;
    RESULTS Results;
		SamplerFactory2D& samplerFact;
    std::ofstream OutputFile;
		Interface* display;
};

//----------------------------------------------------------------------------

