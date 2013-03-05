#pragma once

#include "bamcp/bauct.h"
#include "envs/simulator.h"
#include "planners/mcp/statistic.h"
#include <fstream>
#include "utils/utils.h"
#include "samplers/samplerFactory.h"

#include "experiment.h"
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

class EXPERIMENT_bauct
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

    EXPERIMENT_bauct(const SIMULATOR& real, const SIMULATOR& simulator, 
        const std::string& outputFile, 
        EXPERIMENT_bauct::PARAMS& expParams, BAUCT::PARAMS& searchParams,
				SamplerFactory& _samplerFact);

    void Run(std::vector<double>& Rhist);
		void RunBandit(std::vector<uint>& Rhist, std::vector<uint>& optArm, uint bestArm);

private:

    const SIMULATOR& Real;
    const SIMULATOR& Simulator;
    EXPERIMENT_bauct::PARAMS& ExpParams;
    BAUCT::PARAMS& SearchParams;
    RESULTS Results;
		
		SamplerFactory& samplerFact;
    std::ofstream OutputFile;
};

//----------------------------------------------------------------------------

