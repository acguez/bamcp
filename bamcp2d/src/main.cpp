#include <iostream>

#include "envs/flat/sq2Dgrid.h"
#include "envs/flat/inf2Dgrid.h"

#include "planners/mcp/experiment.h"
#include "planners/mcp/bamcp2D/bamcp2D.h"
#include "planners/boss/boss/boss.h"
#include "planners/boss/experiment_boss.h"
#include "planners/experiment_baselines.h"
#include "planners/mcp/bfs3/bfs3.h"
#include "planners/mcp/experiment_bfs3.h"

#include "samplers/flat/M1Sampler.h"
#include "samplers/flat/M1SamplerFactory.h"
#include "samplers/flat/M2Sampler.h"
#include "samplers/flat/M2SamplerFactory.h"
#include "samplers/flat/MixtureSampler.h"
#include "samplers/flat/MixtureSamplerFactory.h"
#include "samplers/flat/dummyFactory.h"
#include "samplers/flat/InfGridSamplerFactory.h"

#include "utils/rng.h"
#include "utils/hr_time.h"
#include "utils/utils.h"
#include "utils/anyoption.h"

#include <boost/foreach.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/lexical_cast.hpp>

#include "viz/interface.h"

#if defined(GX)
	#include "viz/graphics2d.h"
	#include <GL/glfw.h>
#endif

using namespace boost::numeric::ublas;
using namespace boost;

#include <sstream>
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

//Seed for rng
ulong seed = 0;

bool DDEBUG = false;


//Algorithms FLAG 
// Any number can be set
//---------------
bool RAND = false;
bool BAMCP2D = false;
size_t K = 10;
int MCTSDM = 0; //MCTS Display mode
int MCTSRM = 0; //MCTAS Rollout mode

//MCTS RT (reuse (sub)tree?) parameter
bool MCTSRT = false;
//Default exploration constant
double MCTSEC = 1;

bool BFS3 = false;
uint BFS3C = 10;
uint BFS3D = 100;

bool BOSS = false;
size_t BOSSB = 1;

bool TRUEVI = false;

bool QLEARNING = false;
double QLEARNINGe = 0.1;

double Timeout = -1;

//Inf grid parameters
double BETAR0 = 0.5;
double BETAR1 = 0.5;
double BETAC0 = 1;
double BETAC1 = 3;

double BETAR0P = 0.5;
double BETAR1P = 0.5;
double BETAC0P = 1;
double BETAC1P = 3;



//Environment flag
// Only one should be set
uint ENV = 0;
// 0 - Random
// 1 - M1 Grid  
// 2 - M2 Grid  
// 3 - Mixture Rich/poor quadrant - mode 0
// 5 - Mixture Rich/poor quadrant - mode 1 (2 rewards AT)
// 7 - Mixture Rich/poor quadrant - mode 3 (2 rewards AT + constant mask)
// 9 - Inf grid

//input MDP filename
std::string fENV = "last_MDP";
bool ENV_LOAD = false;

bool GEN_ENVS = false;
bool LOAD_AND_DISPLAY_ENVS = false;
uint NUM_ENVS = 1;

uint FACT = 0;
// 0 - ? 
// 1 - M1 sample maze
// 2 - M2 sample maze
// 3 - Mixture - mode 0
// 4 - Lazy Mixture - mode 0
// 5 - Mixture - mode 1
// 6 - Lazy Mixture - mode 1 
// 7 - Mixture - mode 2
// 8 - Lazy Mixture - mode 2 
// 9 - Lazy Inf Grid

// 100 - Dummy Sampler Factory - Only outputs true environment

//Graphics on?
bool VIZ = false;
uint sqsz = 18; //Square size

//Read command-line arguments using the anyoption library
//(refer to anyoption.h)
//A bit ugly, but does the job.
void readCLArgs(int argc, char* argv[])
{
  AnyOption *opt = new AnyOption();
  opt->setVerbose();
  opt->autoUsagePrint(true);

  opt->addUsage( "" );
  opt->addUsage( "Usage: " );
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


	opt->setFlag("help", 'h'); opt->setFlag("DDEBUG"); opt->setOption("N");
	opt->setOption("T"); opt->setOption("S"); opt->setOption("A");
	opt->setOption("gamma"); opt->setOption("f"); opt->setOption("seed");
 opt->setFlag("BAMCP2D"); opt->setOption("K");
	opt->setOption("MCTSRM"); opt->setOption("MCTSDM");opt->setFlag("RAND");
	opt->setOption("ENV"); opt->setOption("MCTSEC"); opt->setFlag("MCTSRT");
	opt->setOption("FACT"); opt->setFlag("BOSS"); opt->setOption("BOSSB");
	opt->setOption("f0"); opt->setFlag("VIZ"); opt->setOption("Timeout");
	opt->setFlag("GEN_ENVS"); opt->setFlag("LDENVS");
	opt->setOption("NUM_ENVS");opt->setFlag("TRUEVI");
	opt->setFlag("BFS3"); opt->setOption("BFS3C"); opt->setOption("BFS3D");	
	opt->setFlag("QLEARNING"); opt->setOption("QLEARNINGe");
	opt->setOption("BETAC0");opt->setOption("BETAC0P");
	opt->setOption("BETAC1");opt->setOption("BETAC1P");
	opt->setOption("BETAR0");opt->setOption("BETAR0P");
	opt->setOption("BETAR1");opt->setOption("BETAR1P");

	opt->processCommandArgs(argc, argv);

  if( opt->getFlag( "help" ) || opt->getFlag( 'h' ) ) {
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
	if( opt->getValue("T") != NULL)
		T = utils::StringToInt(opt->getValue("T"));
	if( opt->getValue("seed") != NULL)
		seed = (ulong) utils::StringToInt(opt->getValue("seed"));
	if(opt->getFlag("BAMCP2D"))
		BAMCP2D = true;
	if(opt->getValue("MCTSEC") != NULL)
		MCTSEC = utils::StringToInt(opt->getValue("MCTSEC"));
	if(opt->getValue("MCTSRM") != NULL)
		MCTSRM = utils::StringToInt(opt->getValue("MCTSRM"));
	if(opt->getValue("MCTSDM") != NULL)
		MCTSDM = utils::StringToInt(opt->getValue("MCTSDM"));
	if(opt->getFlag("MCTSRT"))
		MCTSRT = true;
	if(opt->getValue("K") != NULL)
		K = utils::StringToInt(opt->getValue("K"));
	if(opt->getFlag("BOSS"))
		BOSS = true;
	if(opt->getValue("BOSSB") != NULL)
		BOSSB = utils::StringToInt(opt->getValue("BOSSB"));
	if(opt->getFlag("RAND"))
		RAND = true;
	if(opt->getValue("ENV"))
		ENV = utils::StringToInt(opt->getValue("ENV"));
	if(opt->getValue("FACT"))
		FACT = utils::StringToInt(opt->getValue("FACT"));
	if(opt->getValue("f0") != NULL){
		fENV = opt->getValue("f0");	
		ENV_LOAD = true;
	}
	if(opt->getFlag("GEN_ENVS")){
		GEN_ENVS = true;
		fENV = opt->getValue("f0");	
	}
	if(opt->getFlag("LDENVS")){
		LOAD_AND_DISPLAY_ENVS = true;
		fENV = opt->getValue("f0");	
	}
	if(opt->getValue("NUM_ENVS") != NULL)
		NUM_ENVS = utils::StringToInt(opt->getValue("NUM_ENVS"));
	if(opt->getFlag("VIZ"))
		VIZ = true;
	if(opt->getValue("Timeout") != NULL)
		Timeout = utils::StringToFloat(opt->getValue("Timeout"));
	if(opt->getFlag("TRUEVI"))
		TRUEVI = true;
	if(opt->getFlag("BFS3"))
		BFS3 = true;
	if(opt->getValue("BFS3C") != NULL)
		BFS3C = utils::StringToFloat(opt->getValue("BFS3C"));
	if(opt->getValue("BFS3D") != NULL)
		BFS3D = utils::StringToFloat(opt->getValue("BFS3D"));
	if(opt->getFlag("QLEARNING"))
		QLEARNING = true;
	if(opt->getValue("QLEARNINGe") != NULL)
		QLEARNINGe = utils::StringToFloat(opt->getValue("QLEARNINGe"));
	if(opt->getValue("BETAC0") != NULL)
		BETAC0 = utils::StringToFloat(opt->getValue("BETAC0"));
	if(opt->getValue("BETAC0P") != NULL)
		BETAC0P = utils::StringToFloat(opt->getValue("BETAC0P"));
	else
		BETAC0P = BETAC0;
	if(opt->getValue("BETAC1") != NULL)
		BETAC1 = utils::StringToFloat(opt->getValue("BETAC1"));
	if(opt->getValue("BETAC1P") != NULL)
		BETAC1P = utils::StringToFloat(opt->getValue("BETAC1P"));
	else
		BETAC1P = BETAC1;
	if(opt->getValue("BETAR0") != NULL)
		BETAR0 = utils::StringToFloat(opt->getValue("BETAR0"));
	if(opt->getValue("BETAR0P") != NULL)
		BETAR0P = utils::StringToFloat(opt->getValue("BETAR0P"));
	else
		BETAR0P = BETAR0;
	if(opt->getValue("BETAR1") != NULL)
		BETAR1 = utils::StringToFloat(opt->getValue("BETAR1"));
	if(opt->getValue("BETAR1P") != NULL)
		BETAR1P = utils::StringToFloat(opt->getValue("BETAR1P"));
	else
		BETAR1P = BETAR1;
	delete opt;
}

void printShortParams(){
	std::cout << "gam=" << gammadisc << " steps " << T << " N " << N  << " A " << A << std::endl;
}
void printParams(){
	std::cout << "MDP Exploration -- Parameters:" << std::endl;
	std::cout << "Number of steps:          " << T << std::endl;
	std::cout << "Number of trials per MDP: " << N << std::endl;
	std::cout << "Number of states:         " << S << std::endl;
	std::cout << "Number of actions:        " << A << std::endl;
	std::cout << "Discount factor:          " << gammadisc << std::endl;
		std::cout << "------------------------------" << std::endl;
	std::cout << "SVN Revision                  " << utils::getSvnRevision()
		<< std::endl;
	std::cout << "Seed                          " << seed << std::endl;
	std::cout << ".............................." << std::endl;
}

//Generate a bunch of environments using the sampler 
void gen_envs(){
	if(ENV > 2 && ENV != 9){
		SIMULATOR2D* real = 0;
		Sampler2D* sampler = 0;
		MixtureSamplerFactory* fact = 0;
		if(ENV == 3)
			fact = new MixtureSamplerFactory(false,0);
		else if(ENV == 5)
			fact = new MixtureSamplerFactory(false,1);
		else if(ENV == 7)
			fact = new MixtureSamplerFactory(false,2);

		for(uint i=0; i < NUM_ENVS; ++i){
			//Reinitialize the sampler each time to get independent samples (could generate from gen model instead)
			sampler = fact->getMDPSampler(gammadisc); 
			do{
				real = sampler->updateMDPSample();
			}while(!((SqGrid*)real)->checkLegalStart());//reject envs with invalid start states
			std::stringstream tmpstream;
			tmpstream << fENV << i;
			std::cout << tmpstream.str() << std::endl;
			((SqGrid*)real)->Save(tmpstream.str());
		}
	}else if(ENV == 9){
		double* betap_row_real = new double[2];
		double* betap_col_real = new double[2];

		betap_row_real[0] = BETAR0;
		betap_row_real[1] = BETAR1;
		betap_col_real[0] = BETAC0;
		betap_col_real[1] = BETAC1;

		for(uint i=0; i < NUM_ENVS; ++i){
			InfGrid* real = new InfGrid(gammadisc,betap_col_real,betap_row_real);
			std::stringstream tmpstream;
			tmpstream << fENV << i;
			std::cout << tmpstream.str() << std::endl;
			((InfGrid*)real)->Save(tmpstream.str());
			delete real;
		}

		delete[] betap_row_real;
		delete[] betap_col_real;
	}
	exit(1);
}

//Load NUM_ENVS envs and display them all in a grid fashion
void load_and_display_envs(){
#if defined(GX)
	std::stringstream tmpstream;
	tmpstream << fENV << 0;
	std::cout << tmpstream.str() << std::endl;
	SIMULATOR2D* real = 0;
	real = new SqGrid(0,0,4,gammadisc);
	((SqGrid*)real)->Load(tmpstream.str());
	
	sqsz = 4;
	glfwInit();
	uint X = real->getSizeX();
	uint Y = real->getSizeY();
	uint sx = std::max(1.0,sqrt(NUM_ENVS));
	uint sy = ceil((double)NUM_ENVS/sx);
	G2D::Initialize(sqsz*sx*X,sqsz*sy*Y);
	
	glfwSwapBuffers();		//Swap buffers and update input	
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	real->render(0,0,sqsz);
	double offset = X*sqsz;
	double xx = 0;
	double yy = 0;
	for(uint i = 1; i < NUM_ENVS; ++i){
		std::stringstream tmpstream;
		tmpstream << fENV << i;
		std::cout << tmpstream.str() << std::endl;
		((SqGrid*)real)->Load(tmpstream.str());

		if(xx+offset < offset*sx){
			xx += offset;
		}
		else{ xx = 0; yy += Y*sqsz;}
		real->render(xx,yy,sqsz);
	}
	
	glfwSwapBuffers();
	sleep(50);
#endif
	exit(1);
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

	if(GEN_ENVS)
		gen_envs();
	if(LOAD_AND_DISPLAY_ENVS)
		load_and_display_envs();

	SIMULATOR2D* real = 0;
	Sampler2D* sampler = 0;
	
	//Generative Beta parameters (for inf grid)
	double* betap_row_real = new double[2];
	double* betap_col_real = new double[2];
	
	betap_row_real[0] = BETAR0; 
	betap_row_real[1] = BETAR1; 
	betap_col_real[0] = BETAC0; 
	betap_col_real[1] = BETAC1; 

	//Prior Beta parameters (for Inf grid)
	double* betap_row_prior = new double[2];
	double* betap_col_prior = new double[2];
	betap_row_prior[0] = BETAR0P; 
	betap_row_prior[1] = BETAR1P; 
	betap_col_prior[0] = BETAC0P; 
	betap_col_prior[1] = BETAC1P; 

	//Select environment
	if(ENV_LOAD && ENV != 9){
		real = new SqGrid(0,0,4,gammadisc);
		((SqGrid*)real)->Load(fENV);
	}
	else if(ENV==0){			
			assert(0);	
			//real = new BasicMDP(S,A,gammadisc,r,rsas, counts);
			//((BasicMDP*)real)->saveMDP("last_mdp");
	}
	else if(ENV == 1){ //M1 sample maze
		M1SamplerFactory* fact = new M1SamplerFactory();
		sampler = fact->getMDPSampler(gammadisc);
		real = sampler->updateMDPSample();
		//((BasicMDP*)real)->saveMDP("last_mdp");
	}
	else if(ENV == 2){ //M2 sample maze
		M2SamplerFactory* fact = new M2SamplerFactory();
		sampler = fact->getMDPSampler(gammadisc);
		real = sampler->updateMDPSample();
	}
	else if(ENV == 3){ //Mixture sample maze
		MixtureSamplerFactory* fact = new MixtureSamplerFactory(false,0);
		sampler = fact->getMDPSampler(gammadisc);
		real = sampler->updateMDPSample();
		((SqGrid*)real)->Save("last_mdp");
	}
	else if(ENV == 5){ //Mixture sample maze
		MixtureSamplerFactory* fact = new MixtureSamplerFactory(false,1);
		sampler = fact->getMDPSampler(gammadisc);
		real = sampler->updateMDPSample();
		((SqGrid*)real)->Save("last_mdp");
	}
	else if(ENV == 7){ //Mixture sample maze
		MixtureSamplerFactory* fact = new MixtureSamplerFactory(false,2);
		sampler = fact->getMDPSampler(gammadisc);
		real = sampler->updateMDPSample();
		((SqGrid*)real)->Save("last_mdp");
	}
	else if(ENV == 9){ //Inf grid
		std::cout << "Creating Inf grid with params: " << betap_col_real[0]
			<< " " << betap_col_real[1] << " " << betap_row_real[0] << " " << betap_row_real[1]
			<< std::endl; 
		real = new InfGrid(gammadisc,betap_col_real,betap_row_real);
		if(BETAC0 < 0)	
			((InfGrid*)real)->Load(fENV);
		else
			((InfGrid*)real)->Save("last_mdp");
	}
	//------------------------
	
	Interface* display = 0;


#if defined(GX)
	if(VIZ){ //initialize graphics
		glfwInit();
		G2D::Initialize(sqsz*real->getSizeX()*1.5+10+30, sqsz*real->getSizeY()+50);
		display = new Interface(sqsz,real->getSizeX(),real->getSizeY(),real);
		display->StartRender();
	}
#endif

	std::vector<std::vector<double> > Rhist(N);
	std::vector<std::vector<uint> > Ahist(N);

	SamplerFactory2D *samplerFact = 0;
	//Select Factory
	if(FACT == 0){
		assert(0);	
	}
	else if(FACT == 1){
		samplerFact = new M1SamplerFactory();
	}
	else if(FACT == 2){
		samplerFact = new M2SamplerFactory();
	}
	else if(FACT == 3){
		samplerFact = new MixtureSamplerFactory(false,0);
	}
	else if(FACT == 4){
		samplerFact = new MixtureSamplerFactory(true,0);
	}
	else if(FACT == 5){
		samplerFact = new MixtureSamplerFactory(false,1);
	}
	else if(FACT == 6){
		samplerFact = new MixtureSamplerFactory(true,1);
	}
	else if(FACT == 7){
		samplerFact = new MixtureSamplerFactory(false,2);
	}
	else if(FACT == 8){
		samplerFact = new MixtureSamplerFactory(true,2);
	}
	else if(FACT == 9){
			std::cout << "Creating Inf grid Factory with params: " << betap_col_prior[0]
			<< " " << betap_col_prior[1] << " " << betap_row_prior[0] << " " << betap_row_prior[1]
			<< std::endl; 

		samplerFact = new InfGridSamplerFactory(betap_col_prior,betap_row_prior);
	}
	else if(FACT == 100){
		samplerFact = new DummyFactory(real);
	}
	CStopWatch timer;
	
	//Run selected planning algorithm
	
	if(RAND || TRUEVI || QLEARNING){
		EXPERIMENT_BASELINES::PARAMS bl_params;
		bl_params.NumSteps = T;
		bl_params.TimeOut = 32000;
		
		if(RAND)
			bl_params.Mode = 0;
		else if(TRUEVI)
			bl_params.Mode = 1;
		else
			bl_params.Mode = 2;
		
		bl_params.epsilon = QLEARNINGe;
		EXPERIMENT_BASELINES exp_bl(*real,bl_params);		
		for(uint i=0;i<N;++i){
			Rhist[i].reserve(T);
			exp_bl.Run(Rhist[i]);
		}
		std::string filename_bl(datafilename);
		if(RAND)
			filename_bl.append("_rand");
		else if(TRUEVI)
			filename_bl.append("_fovi");
		else
			filename_bl.append("_ql");

		utils::dumpc(Rhist,filename_bl,T);
	}
	if(BAMCP2D){
		std::cout << std::endl << "**************" << std::endl;
		std::cout << "--BAMCP-- K " << K << " EC " << MCTSEC << std::endl;
		Rhist = std::vector<std::vector<double> >(N);
		Ahist = std::vector<std::vector<uint> >(N);

		BAMCP2D::PARAMS searchParamsMCTS;
		EXPERIMENT::PARAMS expParamsMCTS;
		searchParamsMCTS.MaxDepth = real->GetHorizon(expParamsMCTS.Accuracy, expParamsMCTS.UndiscountedHorizon);
		expParamsMCTS.NumSteps = T;
		searchParamsMCTS.NumSimulations = K;
		if(Timeout > 0)
			searchParamsMCTS.Timeout = Timeout;
		expParamsMCTS.AutoExploration = false;
		expParamsMCTS.TimeOut = 32000;
		searchParamsMCTS.ExplorationConstant = MCTSEC;
		searchParamsMCTS.DisplayMode = MCTSDM;
		searchParamsMCTS.RolloutMode = MCTSRM;
		searchParamsMCTS.ReuseTree = MCTSRT;
	
    std::string filename_bmcp_all(datafilename);
		filename_bmcp_all.append("_bmcp_all");
		EXPERIMENT experiment(*real, *real,filename_bmcp_all, 
				expParamsMCTS,
				searchParamsMCTS,
				*samplerFact,
				display);

		timer.startTimer();
		for(uint i=0;i<N;++i){
			Rhist[i].reserve(T);
			Ahist[i].reserve(T);
			experiment.Run(Rhist[i],Ahist[i]);
			samplerFact->reset();
		}
		timer.stopTimer();
		std::string filename_bmcp(datafilename);
		filename_bmcp.append("_bmcp");
		utils::dumpc(Rhist,filename_bmcp,T);
		filename_bmcp.append("A");
		utils::dumpc(Ahist,filename_bmcp,T);
		std::cout << "Time: " << timer.getElapsedTime()/(double)N  << " s";
		utils::append(timer.getElapsedTime()/(double)N,filename_bmcp_all);
	}

	if(BOSS){
		assert(FACT != 4 && FACT != 6 && FACT != 8 && FACT != 9);
		std::cout << std::endl << "**************" << std::endl;
		std::cout << "--BOSS-- BOSSB " << BOSSB << std::endl;
		Rhist = std::vector<std::vector<double> >(N);

		BOSS::PARAMS searchParamsBOSS;
		EXPERIMENT_BOSS::PARAMS expParamsBOSS;
		expParamsBOSS.NumSteps = T;
		searchParamsBOSS.K = K;
		searchParamsBOSS.B = BOSSB;
		searchParamsBOSS.Timeout = Timeout;
	//	searchParamsBOSS.epsilon = 0.001;
		expParamsBOSS.AutoExploration = false;
		expParamsBOSS.TimeOut = 32000;
		std::string filename_boss_all(datafilename);
		filename_boss_all.append("_boss_all");
		EXPERIMENT_BOSS experiment_boss(*real, *real,filename_boss_all, 
				expParamsBOSS,
				searchParamsBOSS,
				*samplerFact,
				display);

		timer.startTimer();
		for(uint i=0;i<N;++i){
			Rhist[i].reserve(T);
			experiment_boss.Run(Rhist[i]);
			samplerFact->reset();
		}
		timer.stopTimer();
		std::string filename_boss(datafilename);
		filename_boss.append("_boss");
		utils::dumpc(Rhist,filename_boss,T);
		std::cout << "Time: " << timer.getElapsedTime()/(double)N  << " s";
		utils::append(timer.getElapsedTime()/(double)N,filename_boss_all);
	}
	if(BFS3){
		std::cout << std::endl << "**************" << std::endl;
		std::cout << "--BFS3-- BFS3C " << BFS3C << " BFS3D " << BFS3D << "#Sims "<< K << std::endl;
		Rhist = std::vector<std::vector<double> >(N);

		BFS3::PARAMS searchParamsBFS3;
		EXPERIMENT_BFS3::PARAMS expParamsBFS3;
		expParamsBFS3.NumSteps = T;
		searchParamsBFS3.N = K;
		searchParamsBFS3.C = BFS3C;
		searchParamsBFS3.D = BFS3D;
		searchParamsBFS3.Timeout = Timeout;
		//FIXME!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//This should be obtained from simulator
		if(ENV > 2 && ENV != 9)
			searchParamsBFS3.Vmin = -PREDCOSTCONST/(1-gammadisc);
		else
			searchParamsBFS3.Vmin = 0;
		searchParamsBFS3.Vmax = 1.5/(1-gammadisc);
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	  if(ENV == 9){
      std::cout << "Error: BFS3 not implemented to deal with Infinite Grid domain." << std::endl;
      exit(1);
    }
		searchParamsBFS3.gamma = gammadisc;
		std::cout << "VMin " << searchParamsBFS3.Vmin << " VMax " << searchParamsBFS3.Vmax << std::endl;
		expParamsBFS3.TimeOut = 32000;
		std::string filename_bfs3_all(datafilename);
		filename_bfs3_all.append("_bfs3_all");
		EXPERIMENT_BFS3 experiment_bfs3(*real, *real,filename_bfs3_all, 
				expParamsBFS3,
				searchParamsBFS3,
				*samplerFact,
				display);

		timer.startTimer();
		for(uint i=0;i<N;++i){
			Rhist[i].reserve(T);
			experiment_bfs3.Run(Rhist[i]);
			samplerFact->reset();
		}
		timer.stopTimer();
		std::string filename_bfs3(datafilename);
		filename_bfs3.append("_bfs3");
		utils::dumpc(Rhist,filename_bfs3,T);
		std::cout << "Time: " << timer.getElapsedTime()/(double)N  << " s";
		utils::append(timer.getElapsedTime()/(double)N,filename_bfs3_all);
	}

	delete[] betap_col_real;
	delete[] betap_row_real;
	delete[] betap_col_prior;
	delete[] betap_row_prior;
	delete sampler;
	delete display;	
}









