#include "boss.h"

#include <math.h>
#include "utils/utils.h"
#include <algorithm>

#include "planners/MDPutils.h" 

#include "samplers/flat/sampler2D.h"
#include "envs/flat/simulator2D.h"
#include "samplers/flat/samplerFactory2D.h"


#if defined(GX)
#include "viz/interface.h"
#include "viz/font.h"
#include <GL/glfw.h>
#endif

#include "boost/timer.hpp"

using namespace std;
using namespace UTILS;

//-----------------------------------------------------------------------------

BOSS::PARAMS::PARAMS()
:   Verbose(0),
		K(20),
		B(5),
		epsilon(0.0001),
		Timeout(1.0)
{
}

BOSS::BOSS(const SIMULATOR2D& simulator, const PARAMS& params,
					 SamplerFactory2D& sampFact,
					 State2D* startState,
					 Interface* _display)
:   Simulator(simulator),
    Params(params),
		SampFact(sampFact),
		display(_display)
{
		A = Simulator.GetNumActions();
		
		X = Simulator.getSizeX();
		Y = Simulator.getSizeY();

		//Not really the size of the state space, but it is what we need here
		S = X*Y;
		SA = S*A;
		
//		Am = A*Params.K;

		qcounts = new uint[SA];
		std::fill(qcounts,qcounts+SA,0);

		mdp = 0;
		RLPI =  0;

		sumReward = 0;
		step = 0;

		SampFact.firstObs(startState);
		
		MDPSampler = 0;
		do_sample = true;
}

BOSS::~BOSS()
{
	delete[] qcounts;
	if(mdp)
		mdp->deleteRLPI(RLPI);
}

bool BOSS::Update(State2D* state, uint action, State2D* observation, double reward)
{
		//Update posterior
		qcounts[(state->x*Y+state->y)*A+action] += 1;
		SampFact.update(state,action,observation);

		sumReward += reward;
		//Check resampling criterion 
		//std::cout << qcounts[(state->x*Y+state->y)*A+action] << " " << Params.B << std::endl;
		if(qcounts[(state->x*Y+state->y)*A+action] == Params.B)
			do_sample = true;
	

		return true;
}
/*
void BOSS::createMergedModel(){

	for(size_t i=0; i<S; ++i){
		uint iSAm = i*SAm;
		uint iSA = i*SA;
		for(size_t a=0; a<A; ++a){
			//Get a sampler to sample K T(s,a,.) parameters
			Sampler* TParamSampler = SampFact.getTransitionParamSampler(counts+iSA+a*S,i,a,S);
			uint kA = 0;	
			for(size_t k=0; k<Params.K; ++k){
				TParamSampler->getNextTParamSample(Pm+iSAm+(a+kA)*S);
				kA+=A;
			}
			delete TParamSampler;
		}
	}
}
*/

uint BOSS::SelectAction(State2D* state)
{
		boost::timer timer;

		step = step + 1;
		uint a;
		if(do_sample){
			if(mdp)
				mdp->deleteRLPI(RLPI);
			RLPI = 0;
			//Sample 1 MDP
			if(MDPSampler)
				delete MDPSampler;
			MDPSampler = SampFact.getMDPSampler(Simulator.GetDiscount());
			
			mdp = MDPSampler->updateMDPSample();
			do_sample = false;
		}
		//Use computed policy from the merged model	
		a = mdp->getRLPI(RLPI,state,std::max(0.0,Params.Timeout-timer.elapsed()));
		//std::cout << a << std::endl;		
		
		if(display)
			Display(state);

		//a = mdp-RLPI[state]%A;
		
		return a;
}

void BOSS::Display(State2D* currentState){
#if defined(GX)
	display->StartRender();
	display->drawAgent(currentState->x,currentState->y);
	if(currentState->wait >= 0){
		display->drawPredator(currentState->x2,currentState->y2);
	}
	double sx1,sy1,offset;
	
	//Display observations ----------------------
	display->getScreenStartPosition3(sx1,sy1,offset);
	SampFact.render(sx1,sy1,offset);
	//-------------------------------------------

	//Display current sample --------------------
	display->getScreenStartPosition4(sx1,sy1,offset);
	mdp->render(sx1,sy1,offset);	
	//-------------------------------------------
	

	//Display Info ------------------------------
	int ssize = sprintf(dispstr,"Step: %d", step);
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0,1.0,1.0);
	glPushMatrix();
	glTranslated(5.0,5.0,0.0);
	display->font->drawString(dispstr,ssize);
	glTranslated(150.0,0.0,0.0);
	ssize = sprintf(dispstr,"Sum rewards: %.1f", sumReward);
	display->font->drawString(dispstr,ssize);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	//===========================================
	
	display->StopRender();
#endif
}

//-----------------------------------------------------------------------------


