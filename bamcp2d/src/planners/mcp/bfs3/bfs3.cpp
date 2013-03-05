#include "bfs3.h"
#include <math.h>
#include "utils/utils.h"
#include <algorithm>

#include <map>
#include "samplers/flat/sampler2D.h"
#include "envs/flat/simulator2D.h"
#include "samplers/flat/samplerFactory2D.h"

#include "node.h"

#include "viz/interface.h"

#if defined(GX)
#include "viz/font.h"
#include <GL/glfw.h>
#endif

#include "boost/timer.hpp"


using namespace std;
using namespace UTILS;

//-----------------------------------------------------------------------------

BFS3::PARAMS::PARAMS()
:   Verbose(0),
    D(100), //Max Depth
    C(10),  //Branching factor
		N(10),  //Number of trajectories
		Timeout(1.0),
		gamma(0.95),
		Vmin(0),
		Vmax(1/(1-0.95))
{
}

BFS3::BFS3(const SIMULATOR2D& simulator, const PARAMS& params, SamplerFactory2D& sampFact,
		State2D* startState, Interface* _display)
:   Params(params),
		Simulator(simulator),
		SampFact(sampFact),
		display(_display),
    TreeDepth(0)
{

		SampFact.firstObs(startState);
		SampFact_IMG = SampFact.createNewClone();
		SampFact_IMG->Clone(&SampFact);

    VNODE3::NumChildren = Simulator.GetNumActions();
		Root = 0;
		A = Simulator.GetNumActions();

    VNODE3Pc::NumC = Simulator.GetNumC();
		QNODE3::ParamC = Params.C;

		state2 = new State2D(Simulator.GetNumC());
		observation = new State2D(Simulator.GetNumC());

		assert(Params.gamma == Simulator.GetDiscount());

		MDPSampler = 0;
		step = 0;
		sumRewards = 0;
}

BFS3::~BFS3()
{
	VNODE3Pc::FreeAll();
	VNODE3::FreeAll();
	if(MDPSampler)
		delete MDPSampler;
	delete SampFact_IMG;
}

bool BFS3::Update(State2D* state, uint action, State2D* observation, double reward)
{
    //History.Add(action, observation);
	
		//Update posterior
		SampFact.update(state,action,observation);
		sumRewards += reward; 
		std::cout << "|" << std::flush;
		return true;
}

uint BFS3::SelectAction(State2D* state)
{

	//int historyDepth = History.Size();

	std::vector<uint> besta;
	double bestq = -std::numeric_limits<double>::max();

	for(uint aa=0; aa < A; ++aa){

		double qa = 0;
		if(MDPSampler)
			delete MDPSampler;
		MDPSampler = SampFact.getMDPSampler(Params.gamma);
		
		double rewardtmp;

		
		for(uint c=0; c < Params.C; ++c){
			
			TreeDepth = 0;
			PeakTreeDepth = 0;
			
			//Sample transition
			SIMULATOR2D* mdp = MDPSampler->updateMDPSample_ind();
			mdp->Step(state,aa,state2,rewardtmp);

			if (Params.Verbose >= 2)
			{
				cout << "Starting simulation in state " << state2->x << " " << state2->y << endl;

			}
			//MTS using FSSS in belief-MDP
			double totalReward = FSSS(state,aa,state2);

			qa += (rewardtmp + Params.gamma*totalReward)/Params.C;	
			
			//History.Truncate(historyDepth);
			// Delete old tree and create new root
			VNODE3::FreeAll();
			VNODE3Pc::FreeAll();
			Root = 0;
		}

		if(qa > bestq){
			bestq = qa;
			besta.clear();
			besta.push_back(aa);
		}
		else if(qa == bestq)
			besta.push_back(aa);


		if (Params.Verbose >= 2)
			cout << "Reward for action " << aa << " = " << qa << endl;

	}
	step++;
	if(display)
		Display(state);
	return besta[rand() % besta.size()];	
}

double BFS3::FSSS(State2D* prevs, uint aa, State2D* state){
	
	for(uint n=0;n<Params.N;++n){
		//Make a copy of sampler factory (containing posterior) in SampFact_IMG
		//so that we can update SampFact_IMG safely during rollout
		SampFact_IMG->Clone(&SampFact);
		SampFact_IMG->update(prevs,aa,state);
		FSSSRollout(Root,state,0);

	}
	//return max_a U(s,a) at Root
	return Root->Us;
}


void BFS3::FSSSRollout(VNODE3*& vnode, State2D* state, uint depth){
	if(depth == Params.D)
		return;
	if(!vnode){
		vnode = ExpandNode(state);
		bellmanBackup(vnode, -1, state);
	}
	uint a = vnode->bestA();
	QNODE3& qnode = vnode->Child(a);
	//Select transition optimisticaly
	VNODE3Pc* nextVNode = qnode.selectPath(Params.Vmax,Params.Vmin);

	if(Params.Verbose >= 2){
		std::cout << "D: " << depth << " a: " << a << " sp: " 
			<< nextVNode->state->x << " " << nextVNode->state->y << std::endl; 
	}

	//Update posterior
	SampFact_IMG->update(state,a,nextVNode->state);
	FSSSRollout(nextVNode->v,nextVNode->state,depth+1);
	bellmanBackup(vnode, a, state);
}

//TODO:could avoid recomputing constant parts of the sum
//that have not changed for the last rollout
void BFS3::bellmanBackup(VNODE3* vnode, int action, State2D* s){

	double maxU = Params.Vmin;
	double maxL = Params.Vmin;
	

	for(int aa=0;aa<vnode->NumChildren;++aa){
		QNODE3& qnode = vnode->Children[aa];
		qnode.Usa = 0;
		qnode.Lsa = 0;
		
		for(uint i=0; i < qnode.Children.size(); ++i){
			if(qnode.Children[i]->v == 0){
				qnode.Usa += (qnode.Children[i]->reward+Params.gamma*Params.Vmax)*qnode.Children[i]->count/qnode.Count;
				qnode.Lsa += (qnode.Children[i]->reward+Params.gamma*Params.Vmin)*qnode.Children[i]->count/qnode.Count;
				//std::cout << qnode.Children[i]->reward << " " << Params.gamma << " " << Params.Vmax << " "
					//<< qnode.Children[i]->count << " " << qnode.Count << std::endl;
					//std::cout << "Not explored: " << s->x << " " << s->y << " " << qnode.Children[i]->count << " " << qnode.Usa << " " << qnode.Lsa << std::endl;
			}else{
				qnode.Usa += (qnode.Children[i]->reward+Params.gamma*qnode.Children[i]->v->Us)*qnode.Children[i]->count/qnode.Count;
				qnode.Lsa += (qnode.Children[i]->reward+Params.gamma*qnode.Children[i]->v->Ls)*qnode.Children[i]->count/qnode.Count;
					//std::cout << "Explored: " << s->x << " " << s->y << " " << qnode.Children[i]->count << " " << qnode.Usa << " " << qnode.Lsa << std::endl;
			}
		}

		if(qnode.Usa > maxU)
			maxU = qnode.Usa;
		if(qnode.Lsa > maxL)
			maxL = qnode.Lsa;
		
		//std::cout << aa << " " << qnode.Usa << " " << qnode.Lsa << std::endl;
	}
	vnode->Us = maxU;
	vnode->Ls = maxL;

}

VNODE3* BFS3::ExpandNode(State2D* state) 
{
	VNODE3* vnode = VNODE3::Create();
	vnode->Us = Params.Vmax;
		vnode->Ls = Params.Vmin; 
		vnode->SetChildren(Params.Vmax,Params.Vmin);

		if (Params.Verbose >= 2)
    {
        //cout << "Expanding node: ";
        //History.Display(cout);
        //cout << endl;
    }
		//Sample transitions

	for(uint aa=0; aa < A; ++aa){

		if(MDPSampler)
			delete MDPSampler;
		MDPSampler = SampFact_IMG->getMDPSampler(Params.gamma);

		for(uint c=0; c < Params.C; ++c){
			SIMULATOR2D* mdp = MDPSampler->updateMDPSample_ind(); //Get a new independent MDP sample
			//Sample transition
			mdp->Step(state,aa,observation,reward);
			//Add node for sampled successor state
			vnode->Child(aa).Add(observation,reward);
		}
	}
	return vnode;
}

void BFS3::Display(State2D* currentState){
#if defined(GX)
	
	display->StartRender();
	display->drawAgent(currentState->x,currentState->y);
	if(currentState->wait >= 0){
		display->drawPredator(currentState->x2,currentState->y2);
	}
	//Display observations ----------------------
	double sx1,sy1,offset;
	display->getScreenStartPosition3(sx1,sy1,offset);
	SampFact.render(sx1,sy1,offset);
	//-------------------------------------------


	//Display Info ------------------------------
	
	int ssize = sprintf(dispstr,"Step: %d", step);
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0,1.0,1.0);
	glPushMatrix();
	glTranslated(5.0,5.0,0.0);
	display->font->drawString(dispstr,ssize);
	glTranslated(150.0,0.0,0.0);
	ssize = sprintf(dispstr,"Sum rewards: %.1f", sumRewards);
	display->font->drawString(dispstr,ssize);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	//===========================================
	display->StopRender();


#endif
}


//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
