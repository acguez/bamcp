#pragma once

#include "envs/simulator.h"
#include <fstream>
#include "utils/utils.h"

#include "planners/mcp/experiment.h" // TEMP for RESULTS def
#include "planners/mcp/statistic.h"

class EXPERIMENT_RAND
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

    EXPERIMENT_RAND(const SIMULATOR& real, const SIMULATOR& simulator, 
        const std::string& outputFile, 
        EXPERIMENT_RAND::PARAMS& expParams);

    void Run(std::vector<double>& Rhist);

private:

    const SIMULATOR& Real;
    const SIMULATOR& Simulator;
    EXPERIMENT_RAND::PARAMS& ExpParams;
    RESULTS Results;
    
		std::ofstream OutputFile;
};


