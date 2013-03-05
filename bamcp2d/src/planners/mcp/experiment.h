#pragma once

#include "bamcp2D/bamcp2D.h"
#include "envs/flat/simulator2D.h"
#include "samplers/flat/samplerFactory2D.h"

#include "planners/statistic.h"
#include <fstream>
#include "utils/utils.h"

#include "viz/interface.h"
//----------------------------------------------------------------------------

struct RESULTS
{
    void Clear();

    STATISTIC Time;
    STATISTIC Reward;
    STATISTIC DiscountedReturn;
    STATISTIC UndiscountedReturn;
};

inline void RESULTS::Clear()
{
    Time.Clear();
    Reward.Clear();
    DiscountedReturn.Clear();
    UndiscountedReturn.Clear();
}

//----------------------------------------------------------------------------

class EXPERIMENT
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
        bool AutoExploration;
        int UndiscountedHorizon;
    };

    EXPERIMENT(SIMULATOR2D& real, SIMULATOR2D& simulator, 
        const std::string& outputFile, 
        EXPERIMENT::PARAMS& expParams, BAMCP2D::PARAMS& searchParams,
				 SamplerFactory2D& samplerFactory,
				 Interface* _display);

    void Run(std::vector<double>& Rhist, std::vector<uint>& Ahist);

private:

    SIMULATOR2D& Real;
    SIMULATOR2D& Simulator;
    EXPERIMENT::PARAMS& ExpParams;
    BAMCP2D::PARAMS& SearchParams;
    RESULTS Results;
		SamplerFactory2D& samplerFact;
    std::ofstream OutputFile;
		Interface* display;
};

//----------------------------------------------------------------------------

