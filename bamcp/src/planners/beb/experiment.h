#pragma once

#include "beb.h"
#include "envs/simulator.h"
#include <fstream>
#include "utils/utils.h"

#include "planners/mcp/experiment.h" // TEMPORARY dependence for RESULTS def
#include "planners/mcp/statistic.h"

class EXPERIMENT_BEB
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
        int UndiscountedHorizon;
        bool AutoExploration;
    };

    EXPERIMENT_BEB(const SIMULATOR& real, const SIMULATOR& simulator, 
        const std::string& outputFile, 
        EXPERIMENT_BEB::PARAMS& expParams, BEB::PARAMS& searchParams,
				SamplerFactory& _samplerFact);

    void Run(std::vector<double>& Rhist);

private:

    const SIMULATOR& Real;
    const SIMULATOR& Simulator;
    EXPERIMENT_BEB::PARAMS& ExpParams;
    BEB::PARAMS& SearchParams;
    RESULTS Results;
		SamplerFactory& samplerFact;
    
		std::ofstream OutputFile;
};


