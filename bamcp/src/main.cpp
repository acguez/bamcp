#include <iostream>

#include "envs/bandit.h"
#include "envs/banditSim.h"
#include "envs/basicMDP.h"
#include "envs/grid.h"
#include "envs/doubleloop.h"
#include "envs/mazedoor.h"
#include "envs/maze.h"

#include "planners/mcp/experiment.h"
#include "planners/mcp/bamcp/bamcp.h"

#include "planners/mcp/experiment_bauct.h"
#include "planners/mcp/bamcp/bauct.h"

#include "planners/mcp/experiment_bfs3.h"
#include "planners/mcp/bfs3/bfs3.h"

#include "planners/boss/experiment.h"
#include "planners/boss/boss/boss.h"

#include "planners/boss/experiment_sboss.h"
#include "planners/boss/sboss/sboss.h"

#include "planners/beb/experiment.h"
#include "planners/beb/beb.h"

#include "planners/experiment_rand.h"

#include "samplers/FDMsamplerFactory.h"
#include "samplers/SFDMsamplerFactory.h"
#include "samplers/LazySamplerFactory.h"
#include "samplers/DoorSamplerFactory.h"

#include "utils/rng.h"
#include "utils/hr_time.h"
#include "utils/utils.h"
#include "utils/anyoption.h"

#include <boost/foreach.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/lexical_cast.hpp>


using namespace boost::numeric::ublas;
using namespace boost;

#include <algorithm>

//output filename
std::string datafilename = "../out/defaultout";

//Number of steps to execute
size_t T = 100;
//Number of trials per MDP
size_t N = 1;

//Number of states
size_t S = 5;
//Number of actions
size_t A = 4;

//Discount factor
double gammadisc = 0.95;

//BOSS K parameter
size_t K = 10;
//BOSS B parameter
size_t B = 0;

//BFS3 C parameter (branching)
size_t BFS3C = 10;
size_t BFS3D = 40;

//BAMCP RB parameter
int MCTSRB = -1;
//Default exploration constant
double MCTSEC = 3;
double MCTSEPS = 0.5;

//sBOSS delta param
double delta = 3.0;

double BEBbeta = 2.5;

//Seed for rng
ulong seed = 0;

bool DDEBUG = false;

double dev = 0.1;

//Algorithms FLAG 
// Any number can be set
//---------------
bool BFS3 = false;
bool RAND = false;
bool SBOSS = false;
bool BOSS = false;
bool BAMCP = false;
bool BAUCT = false;
bool BEB = false;

//Environment input argument
// Only one should be set
uint ENV = 0;
// 0 - Random
// 1 - Grid  (sqrt(S)*sqrt(S)) cond: A==4
// 3 - Doubleloop  cond S==9 A==2 
// 4 - Maze     cond S==264 A==4
// 10 - Maze door    cond S==17 A==4

//input MDP filename (for ENV=0)
std::string fENV = "last_MDP";
bool ENV_LOAD = false;

uint FACT = 0;
// 0 - FDM
// 1 - SFDM  (uses parameters  SFDMbeta and SFDMalpha)
// 2 - Lazy FDM
// 3 - Lazy SFDM (uses parameters  SFDMbeta and SFDMalpha)

double SFDMbeta = 2.0;
double SFDMalpha = 0.2;

double VMax = -1;

//Read command-line arguments using the anyoption library
//(refer to anyoption.h)
void readCLArgs(int argc, char* argv[])
{
  AnyOption *opt = new AnyOption();
  opt->setVerbose();
  opt->autoUsagePrint(true);

  opt->addUsage( "Bayesian RL framework" );
  opt->addUsage( "Usage: Do not believe what is written below, look in main.cpp for ENVS/FACT/ALGOs arguments");
  opt->addUsage( "See README file for examples.");
  opt->addUsage( "" );
  opt->addUsage( " -h  --help  		Prints this help " );
  opt->addUsage( " --f     Output File" );
  opt->addUsage( " --T     Number of exploration steps" );
  opt->addUsage( " --N     Number of trials per MDP" );
	opt->addUsage( " --S     Number of MDP states");
  opt->addUsage( " --A     Number of MDP ations");
  opt->addUsage( " --gamma Dicount factor");
	opt->addUsage( " --seed  Seed for RNG");
	opt->addUsage( "" );


	opt->setFlag("help", 'h'); opt->setFlag("DDEBUG"); opt->setFlag("SR");
	opt->setFlag("SWR"); opt->setFlag("SUR"); opt->setOption("N");
	opt->setOption("T"); opt->setOption("S");
	opt->setOption("A"); opt->setOption("K"); opt->setOption("B");
	opt->setOption("gamma"); opt->setOption("delta");  
	opt->setOption("f"); opt->setOption("seed"); opt->setOption("dev");
	opt->setOption("BFS3C"); opt->setOption("BFS3D"); 
	opt->setOption("MCTSRB");opt->setOption("MCTSEPS");
	opt->setFlag("BOSS"); opt->setFlag("SBOSS"); opt->setFlag("BAMCP"); 
  opt->setFlag("BFS3"); opt->setFlag("BEB");
	opt->setFlag("RAND"); opt->setOption("ENV"); opt->setOption("BEBbeta");
	opt->setOption("MCTSEC");opt->setOption("FACT"); opt->setOption("SFDMbeta");
	opt->setOption("SFDMalpha");opt->setOption("VMax"); opt->setOption("f0");
	opt->setFlag("BAUCT");

  opt->processCommandArgs(argc, argv);

  if( opt->getFlag( "help" ) || opt->getFlag( 'h' ) ){
    opt->printUsage();
    exit(0);
  }
	if(opt->getFlag("DDEBUG"))
		DDEBUG = true;
  if( opt->getValue("f") != NULL)
    datafilename = opt->getValue("f");
  if( opt->getValue("N") != NULL)
		N = utils::StringToInt(opt->getValue("N"));
  if( opt->getValue("S") != NULL)
		S = utils::StringToInt(opt->getValue("S"));
	if( opt->getValue("A") != NULL)
		A = utils::StringToInt(opt->getValue("A"));
	if( opt->getValue("gamma") != NULL)
		gammadisc = utils::StringToFloat(opt->getValue("gamma"));
	if( opt->getValue("delta") != NULL)
		delta = utils::StringToFloat(opt->getValue("delta"));
	if( opt->getValue("T") != NULL)
		T = utils::StringToInt(opt->getValue("T"));
	if( opt->getValue("K") != NULL)
		K = utils::StringToInt(opt->getValue("K"));
	if( opt->getValue("B") != NULL)
		B = utils::StringToInt(opt->getValue("B"));
	if( opt->getValue("seed") != NULL)
		seed = (ulong) utils::StringToInt(opt->getValue("seed"));
	if(opt->getValue("dev") != NULL)
		dev = utils::StringToFloat(opt->getValue("dev"));
	if(opt->getValue("BFS3C") != NULL)
		BFS3C = utils::StringToFloat(opt->getValue("BFS3C"));
	if(opt->getValue("BFS3D") != NULL)
		BFS3D = utils::StringToFloat(opt->getValue("BFS3D"));
	if(opt->getValue("MCTSRB") != NULL)
		MCTSRB = utils::StringToFloat(opt->getValue("MCTSRB"));
	if(opt->getFlag("BOSS"))
		BOSS = true;
	if(opt->getFlag("SBOSS"))
		SBOSS = true;
	if(opt->getFlag("BAMCP"))
		BAMCP = true;
	if(opt->getFlag("BAUCT"))
		BAUCT = true;
	if(opt->getFlag("BFS3"))
		BFS3 = true;
	if(opt->getFlag("BEB"))
		BEB = true;
	if(opt->getFlag("RAND"))
		RAND = true;
	if(opt->getValue("ENV"))
		ENV = utils::StringToInt(opt->getValue("ENV"));
	if(opt->getValue("BEBbeta"))
		BEBbeta = utils::StringToFloat(opt->getValue("BEBbeta"));
	if(opt->getValue("MCTSEC"))
		MCTSEC = utils::StringToFloat(opt->getValue("MCTSEC"));
	if(opt->getValue("SFDMbeta"))
		SFDMbeta = utils::StringToFloat(opt->getValue("SFDMbeta"));
	if(opt->getValue("SFDMalpha"))
		SFDMalpha = utils::StringToFloat(opt->getValue("SFDMalpha"));
	if(opt->getValue("FACT"))
		FACT = utils::StringToInt(opt->getValue("FACT"));
	if(opt->getValue("VMax"))
		VMax = utils::StringToFloat(opt->getValue("VMax"));
	if(opt->getValue("f0") != NULL){
		fENV = opt->getValue("f0");	
		ENV_LOAD = true;
	}
	if(opt->getValue("MCTSEPS")){
		MCTSEPS = utils::StringToFloat(opt->getValue("MCTSEPS"));	
	}
	delete opt;
}

void printShortParams(){
	std::cout << "gam=" << gammadisc << " steps " << T << " N " << N << std::endl; //" S " << S << " A " << A << std::endl;
}
void printParams(){
	std::cout << "MDP Exploration -- Parameters:" << std::endl;
	std::cout << "Number of steps:          " << T << std::endl;
	std::cout << "Number of trials per MDP: " << N << std::endl;
	std::cout << "Number of states:         " << S << std::endl;
	std::cout << "Number of actions:        " << A << std::endl;
	std::cout << "Discount factor:          " << gammadisc << std::endl;
	std::cout << "K:                        " << K << std::endl;
	std::cout << "B:                        " << B << std::endl;
	std::cout << "sBOSS delta               " << delta << std::endl;
	std::cout << "BFS3C                     " << BFS3C << std::endl;
	std::cout << "BFS3D                     " << BFS3D << std::endl;
	std::cout << "MCTSRB                    " << MCTSRB << std::endl;
	std::cout << "VMax                      " << VMax << std::endl;
	std::cout << "------------------------------" << std::endl;
	std::cout << "SVN Revision                  " << utils::getSvnRevision()
		<< std::endl;
	std::cout << "Seed                          " << seed << std::endl;
	std::cout << ".............................." << std::endl;
}


/*
 *  Main, sets up the experiment and dumps results.
 *
 */
int main(int argc, char* argv[]){
	//Read arguments
	readCLArgs(argc,argv);

	if(seed != 0)
		utils::setSeed(seed);

	//printParams();
	printShortParams();

	//-------------------------------------------------------------
	//evalBandits();
	
	//Evaluate different algorithms on an MDP
	/////////////////////////////////////////
	SIMULATOR* real = 0;
	bool rsas; // Reward function R(s,a,s')? otherwise assume R(s,a)
	double* r = 0;
	uint* counts = 0;

	//Select environment
	if(ENV_LOAD){
		real = new BasicMDP(S,A,fENV,gammadisc);
	}
	else if(ENV==0 || ENV == 7 || ENV == 5){
			r = new double[S*A];
			counts = new uint[S*A*S];
			std::fill(counts,counts+S*A*S,0);
			std::fill(r,r+S*A,0);
			if(ENV == 0){
				for(uint a = 0; a < A; ++a)
					r[(S-1)*A+a] = 1;
			}else if(ENV == 7){
				for(uint a = 0; a < A; ++a)
					r[(S-1)*A+a] = 2;
				for(uint a = 0; a < A; ++a)
					r[S/2*A+a] = 1;
			}else if(ENV == 5){
				for(uint s=0; s < S; ++s){
					double rs = utils::rng.rand_closed01();
					for(uint a = 0; a < A; ++a)
						r[s*A+a] = rs; 
				}
			}

			//Rand MDP sampled from prior
			rsas = false;
			real = new BasicMDP(S,A,gammadisc,r,rsas, counts);
			((BasicMDP*)real)->saveMDP("last_mdp");
	}
	else if(ENV == 100){
		S = 3;
		A = 2;
		int SA=S*A;
		rsas = true;
		r = new double[SA*S];
		std::fill(r,r+SA*S,0);
		r[0*SA+0*S+1] = 0.5;
		r[0*SA+1*S+1] = 1;
		r[0*SA+2*S+1] = 0;
		double* T = new double[SA*S];
		double p = utils::rng.rand_closed01();
		std::cout << "**************************************" << std::endl;
		std::cout << "REAL parameter p: " << p << std::endl;
		std::cout << "**************************************" << std::endl;
		std::fill(T,T+S*A*S,0);
		T[0*SA+0*S+1] = 1;
		T[0*SA+1*S+1] = p;
		T[0*SA+1*S+2] = 1-p;
		T[1*SA+0*S+0] = 1;
		T[1*SA+1*S+0] = 1;
		T[2*SA+0*S+0] = 1;
		T[2*SA+1*S+0] = 1;
		real = new BasicMDP(S,A,gammadisc,r,rsas,T);		
	}
	else if(ENV == 6){ //Sparse random MDP
		SamplerFactory* fact = new SFDMsamplerFactory(SFDMalpha,false,SFDMbeta,S,A);
		r = new double[S*A];
		counts = new uint[S*A*S];
		std::fill(counts,counts+S*A*S,0);
		std::fill(r,r+S*A,0);
		for(uint s=0; s < S; ++s){
			double rs = utils::rng.rand_closed01();
			for(uint a = 0; a < A; ++a)
				r[s*A+a] = rs;
		}
		Sampler* sampler = fact->getMDPSampler(counts,S,A,r,false,gammadisc);
		real = sampler->updateMDPSample();
		((BasicMDP*)real)->saveMDP("last_mdp");
		delete fact;
		delete sampler;
	}
	else if(ENV == 1){
		//Grid env
		assert(pow((int)sqrt(S),2) == S);
		assert(A == 4);
		real = new Grid(sqrt(S),gammadisc); 
	}
	else if(ENV == 3){
		//Double loop env
		real = new Dloop(gammadisc);
		assert(S==9 && A==2);
	}
	else if(ENV ==4){
		//Maze env
		real = new Maze(gammadisc);
		assert(S==264 && A==4);
	}
	else if(ENV == 10){
		//Maze env
		real = new MazeDoor(false, gammadisc);
		assert(S==17 && A==4);
	}
	//------------------------
	
	std::vector<std::vector<double> > Rhist(N);
	

	SamplerFactory *samplerFact;
	//Select (Sampler) Factory
	if(FACT == 0){
		samplerFact =  new FDMsamplerFactory(1/(double)S); 
	}
	else if(FACT == 1)
		samplerFact = new SFDMsamplerFactory(SFDMalpha,false,SFDMbeta,S,A);
	else if(FACT == 2){ 
		assert(BOSS == 0 && SBOSS == 0 && BEB == 0 && BFS3 == 0);
		FDMsamplerFactory* samplerFact2 = new FDMsamplerFactory(1/(double)S); 
		samplerFact = new LazySamplerFactory(samplerFact2);
	}
	else{
		assert(BOSS == 0 && SBOSS == 0 && BEB == 0 && BFS3 == 0);
		SFDMsamplerFactory* samplerFact2 = new SFDMsamplerFactory(SFDMalpha,false,SFDMbeta,S,A);	
		samplerFact = new LazySamplerFactory(samplerFact2);
	}

	if(ENV == 10){
		samplerFact = new DoorsamplerFactory(SFDMbeta);   //parameter overload
	}

	//Run selected algorithm(s)
	if(RAND){
    
    EXPERIMENT_RAND::PARAMS expParams;
		expParams.NumSteps = T;
		std::string filename_ran_all(datafilename);
		filename_ran_all.append("_ran_all");
		EXPERIMENT_RAND experiment(*real, *real,filename_ran_all, expParams);

		for(uint i=0;i<N;++i){
			Rhist[i].reserve(T);
			experiment.Run(Rhist[i]);
		}
		std::string filename_ran(datafilename);
		filename_ran.append("_ran");
		utils::dumpc(Rhist,filename_ran,T);
		

	}

		CStopWatch timer;

	if(BAMCP){
		std::cout << std::endl << "**************" << std::endl;
		std::cout << "--BAMCP-- K " << K << " EC " << MCTSEC << std::endl;
		Rhist = std::vector<std::vector<double> >(N);;

		BAMCP::PARAMS searchParamsBAMCP;
		EXPERIMENT::PARAMS expParamsBAMCP;
		searchParamsBAMCP.MaxDepth = real->GetHorizon(expParamsBAMCP.Accuracy, expParamsBAMCP.UndiscountedHorizon);
		expParamsBAMCP.NumSteps = T;
		searchParamsBAMCP.NumSimulations = K;
		expParamsBAMCP.AutoExploration = false;
		expParamsBAMCP.TimeOut = 32000;
		searchParamsBAMCP.ExplorationConstant = MCTSEC;
		searchParamsBAMCP.RB = MCTSRB;
		searchParamsBAMCP.eps = MCTSEPS;
		std::string filename_bmcp_all(datafilename);
		filename_bmcp_all.append("_bmcp_all");
		EXPERIMENT experiment2(*real, *real,filename_bmcp_all, 
				expParamsBAMCP,
				searchParamsBAMCP,
				*samplerFact);

		timer.startTimer();
		for(uint i=0;i<N;++i){
			Rhist[i].reserve(T);
			experiment2.Run(Rhist[i]);
			samplerFact->reset();
		}
		timer.stopTimer();
		std::string filename_bmcp(datafilename);
		filename_bmcp.append("_bmcp");
		utils::dumpc(Rhist,filename_bmcp,T);
		std::cout << "Time: " << timer.getElapsedTime()/(double)N  << " s";
		utils::append(timer.getElapsedTime()/(double)N,filename_bmcp_all);
	}
  if(BAUCT){
    std::cout << std::endl << "**************" << std::endl;
    std::cout << "--BAUCT-- K " << K << " EC " << MCTSEC << std::endl;
    Rhist = std::vector<std::vector<double> >(N);;

    BAUCT::PARAMS searchParamsBAUCT;
    EXPERIMENT_bauct::PARAMS expParamsBAUCT;
    searchParamsBAUCT.MaxDepth = real->GetHorizon(expParamsBAUCT.Accuracy, expParamsBAUCT.UndiscountedHorizon);
    expParamsBAUCT.NumSteps = T;
    searchParamsBAUCT.NumSimulations = K;
    expParamsBAUCT.AutoExploration = false;
    expParamsBAUCT.TimeOut = 32000;
    searchParamsBAUCT.ExplorationConstant = MCTSEC;
    searchParamsBAUCT.eps = MCTSEPS;

    if(ENV == 100){  
      assert(0);	
    }
    std::string filename_bauct_all(datafilename);
    filename_bauct_all.append("_bauct_all");
    EXPERIMENT_bauct experiment_bauct(*real, *real,filename_bauct_all, 
        expParamsBAUCT,
        searchParamsBAUCT,
        *samplerFact);

    timer.startTimer();
    for(uint i=0;i<N;++i){
      Rhist[i].reserve(T);
      experiment_bauct.Run(Rhist[i]);
      samplerFact->reset();
    }
    timer.stopTimer();
    std::string filename_bauct(datafilename);
    filename_bauct.append("_bauct");
    utils::dumpc(Rhist,filename_bauct,T);
    std::cout << "Time: " << timer.getElapsedTime()/(double)N  << " s";
    utils::append(timer.getElapsedTime()/(double)N,filename_bauct_all);
  }

		//------------------------
	if(BFS3){	
		std::cout << std::endl<< "**************" << std::endl;
		std::cout << "--BFS3-- D" << BFS3D << " C " << BFS3C << " N (K/C) " << std::max(1,(int)( K/(BFS3C))) << std::endl;
		Rhist = std::vector<std::vector<double> >(N);;

		BFS3::PARAMS searchParamsBFS3;
		EXPERIMENT_BFS3::PARAMS expParamsBFS3;
		searchParamsBFS3.D = BFS3D; //real->GetHorizon(expParamsBFS3.Accuracy, expParamsBFS3.UndiscountedHorizon);
		searchParamsBFS3.Vmin = 0;
		searchParamsBFS3.Vmax = real->GetRewardRange()/(1-gammadisc);
		searchParamsBFS3.C = BFS3C; 
		searchParamsBFS3.N = std::max(1,(int)( K/(BFS3C)));
		searchParamsBFS3.gamma = gammadisc;
		expParamsBFS3.NumSteps = T;
		expParamsBFS3.TimeOut = 32000;
		std::string filename_bfs3_all(datafilename);
		filename_bfs3_all.append("_bfs3_all");

		EXPERIMENT_BFS3 experiment3(*real, *real,filename_bfs3_all, 
				expParamsBFS3,
				searchParamsBFS3,
				*samplerFact);
		timer.startTimer();
		for(uint i=0;i<N;++i){
			Rhist[i].reserve(T);
			experiment3.Run(Rhist[i]);
			samplerFact->reset();
		}
		timer.stopTimer();
		std::string filename_bfs3(datafilename);
		filename_bfs3.append("_bfs3");
		utils::dumpc(Rhist,filename_bfs3,T);
		std::cout << "Time: " << timer.getElapsedTime()/(double)N  << " s";
		utils::append(timer.getElapsedTime()/(double)N,filename_bfs3_all);
	}
		//------------------------
	if(BOSS){	
		std::cout << std::endl <<  "**************" << std::endl;
		std::cout << "--BOSS-- K " << K << " B " << B << std::endl;
		Rhist = std::vector<std::vector<double> >(N);;

		BOSS::PARAMS searchParamsBOSS;
		EXPERIMENT_BOSS::PARAMS expParamsBOSS;
		searchParamsBOSS.K = K;
		searchParamsBOSS.B = B;
		expParamsBOSS.NumSteps = T;
		expParamsBOSS.TimeOut = 32000;
		std::string filename_BOSS_all(datafilename);
		filename_BOSS_all.append("_BOSS_all");

		EXPERIMENT_BOSS experiment4(*real, *real,filename_BOSS_all, 
				expParamsBOSS,
				searchParamsBOSS,
				*samplerFact);
		timer.startTimer();
		for(uint i=0;i<N;++i){
			Rhist[i].reserve(T);
			experiment4.Run(Rhist[i]);
			samplerFact->reset();
		}
		timer.stopTimer();
		std::string filename_BOSS(datafilename);
		filename_BOSS.append("_BOSS");
		utils::dumpc(Rhist,filename_BOSS,T);
		std::cout << "Time: " << timer.getElapsedTime()/(double)N  << " s";
		utils::append(timer.getElapsedTime()/(double)N,filename_BOSS_all);
	}
		//------------------------
	if(SBOSS){
		std::cout << std::endl <<  "**************" << std::endl;
		std::cout << "--SBOSS-- K " << K << " delta " << delta << std::endl;
		Rhist = std::vector<std::vector<double> >(N);;

		SBOSS::PARAMS searchParamsSBOSS;
		EXPERIMENT_SBOSS::PARAMS expParamsSBOSS;
		searchParamsSBOSS.K = K;
		searchParamsSBOSS.delta = delta;
		expParamsSBOSS.NumSteps = T;
		expParamsSBOSS.TimeOut = 32000;
		std::string filename_SBOSS_all(datafilename);
		filename_SBOSS_all.append("_SBOSS_all");

		EXPERIMENT_SBOSS experiment5(*real, *real,filename_SBOSS_all, 
				expParamsSBOSS,
				searchParamsSBOSS,
				*samplerFact);
		timer.startTimer();
		for(uint i=0;i<N;++i){
			Rhist[i].reserve(T);
			experiment5.Run(Rhist[i]);
			samplerFact->reset();
		}
		timer.stopTimer();
		std::string filename_SBOSS(datafilename);
		filename_SBOSS.append("_SBOSS");
		utils::dumpc(Rhist,filename_SBOSS,T);
		std::cout << "Time: " << timer.getElapsedTime()/(double)N  << " s";
		utils::append(timer.getElapsedTime()/(double)N,filename_SBOSS_all);
	}
		//------------------------
	if(BEB){	
		std::cout << std::endl <<  "**************" << std::endl;
		std::cout << "--BEB-- beta " << BEBbeta << std::endl;
		Rhist = std::vector<std::vector<double> >(N);;

		BEB::PARAMS searchParamsBEB;
		EXPERIMENT_BEB::PARAMS expParamsBEB;
		searchParamsBEB.b = BEBbeta; 
		expParamsBEB.NumSteps = T;
		expParamsBEB.TimeOut = 32000;
		std::string filename_BEB_all(datafilename);
		filename_BEB_all.append("_BEB_all");

		EXPERIMENT_BEB experiment6(*real, *real,filename_BEB_all, 
				expParamsBEB,
				searchParamsBEB,
				*samplerFact);
		timer.startTimer();
		for(uint i=0;i<N;++i){
			Rhist[i].reserve(T);
			experiment6.Run(Rhist[i]);
			samplerFact->reset();
		}
		timer.stopTimer();
		std::string filename_BEB(datafilename);
		filename_BEB.append("_BEB");
		utils::dumpc(Rhist,filename_BEB,T);
		std::cout << "Time: " << timer.getElapsedTime()/(double)N  << " s";
		utils::append(timer.getElapsedTime()/(double)N,filename_BEB_all);
	}
	//delete real;
	if(ENV==0){
		//delete[] r;
		delete[] counts;
	}
	
}

