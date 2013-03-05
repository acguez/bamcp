#pragma once

#include "boss/boss.h"
#include "envs/simulator.h"
#include <fstream>
#include "utils/utils.h"

#include "planners/mcp/experiment.h" // TEMP for RESULTS def
#include "planners/mcp/statistic.h"

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
        double Accuracy;
        int UndiscountedHorizon;
        bool AutoExploration;
    };

    EXPERIMENT_BOSS(const SIMULATOR& real, const SIMULATOR& simulator, 
        const std::string& outputFile, 
        EXPERIMENT_BOSS::PARAMS& expParams, BOSS::PARAMS& searchParams,
				SamplerFactory& _samplerFact);

    void Run(std::vector<double>& Rhist);
		void RunBandit(std::vector<uint>& Rhist, std::vector<uint>& optArm, uint bestArm);

private:

    const SIMULATOR& Real;
    const SIMULATOR& Simulator;
    EXPERIMENT_BOSS::PARAMS& ExpParams;
    BOSS::PARAMS& SearchParams;
    RESULTS Results;
		SamplerFactory& samplerFact;
    
		std::ofstream OutputFile;
};


