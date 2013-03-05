#pragma once

#include "sboss/sboss.h"
#include "envs/simulator.h"
#include <fstream>
#include "utils/utils.h"

#include "planners/mcp/experiment.h" // TEMP for RESULTS def
#include "planners/mcp/statistic.h"

class EXPERIMENT_SBOSS
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

    EXPERIMENT_SBOSS(const SIMULATOR& real, const SIMULATOR& simulator, 
        const std::string& outputFile, 
        EXPERIMENT_SBOSS::PARAMS& expParams, SBOSS::PARAMS& searchParams,
				SamplerFactory& _samplerFact);

    void Run(std::vector<double>& Rhist);
		void RunBandit(std::vector<uint>& Rhist, std::vector<uint>& optArm, uint bestArm);

private:

    const SIMULATOR& Real;
    const SIMULATOR& Simulator;
    EXPERIMENT_SBOSS::PARAMS& ExpParams;
    SBOSS::PARAMS& SearchParams;
    RESULTS Results;
		SamplerFactory& samplerFact;
    
		std::ofstream OutputFile;
};


