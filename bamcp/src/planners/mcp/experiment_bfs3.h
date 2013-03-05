#pragma once

#include "bfs3/bfs3.h"
#include "envs/simulator.h"
#include "statistic.h"
#include <fstream>
#include "utils/utils.h"

#include "experiment.h"
#include "samplers/samplerFactory.h"

//----------------------------------------------------------------------------


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
        int UndiscountedHorizon;
        bool AutoExploration;
    };

    EXPERIMENT_BFS3(const SIMULATOR& real, const SIMULATOR& simulator,
        const std::string& outputFile, 
        EXPERIMENT_BFS3::PARAMS& expParams, BFS3::PARAMS& searchParams,
				SamplerFactory& samplerFactory);

    void Run(std::vector<double>& Rhist);
		void RunBandit(std::vector<uint>& Rhist, std::vector<uint>& optArm, uint bestArm);

private:

    const SIMULATOR& Real;
    const SIMULATOR& Simulator;
    EXPERIMENT_BFS3::PARAMS& ExpParams;
    BFS3::PARAMS& SearchParams;
    RESULTS Results;
		SamplerFactory& samplerFact;

    std::ofstream OutputFile;
};

//----------------------------------------------------------------------------

