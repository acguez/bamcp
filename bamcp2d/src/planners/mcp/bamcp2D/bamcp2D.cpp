#include "bamcp2D.h"
#include <math.h>
#include "utils/utils.h"
#include <algorithm>

#include <iomanip>

#include "node2D.h"

#include "samplers/flat/sampler2D.h"
#include "samplers/flat/samplerFactory2D.h"

#include "planners/MDPutils.h"

#include "viz/interface.h"

#if defined(GX)
#include "viz/font.h"
#include <GL/glfw.h>
#endif

#include "boost/timer.hpp"

using namespace std;
using namespace UTILS;

//-----------------------------------------------------------------------------

BAMCP2D::PARAMS::PARAMS()
:   Verbose(0),
    MaxDepth(200),
    NumSimulations(1000),
    MaxAttempts(0),
    ExpandCount(1),
    ExplorationConstant(1),
		DisplayMode(0),
		RolloutMode(0),
		Timeout(-1),
		ReuseTree(false),
		QLepsilon(2),
		QLalpha(0.2)
{
}

BAMCP2D::BAMCP2D(SIMULATOR2D& simulator, const PARAMS& params,
		SamplerFactory2D& sampFact,
		State2D* startState,
		Interface* _display)
:   Params(params),
		Simulator(simulator), //We use the real simulator only to get #actions etc. 
    TreeDepth(0),
		SampFact(sampFact),
		display(_display)
{
    VNODE2D::NumChildren = Simulator.GetNumActions();

    QNODE2DAT::NumAT = Simulator.GetNumAttributes();
    QNODE2DAT::NumC = Simulator.GetNumC();
    
		Root = ExpandNode(startState); //Simulator.CreateStartState());
		
		SampFact.firstObs(startState);

		A = Simulator.GetNumActions();
		NumAT = Simulator.GetNumAttributes();
		NumC = Simulator.GetNumC();	
		
		step = 0;
		sumRewards = 0;

//		meand.open("BMCP_meandepth",std::ios_base::app);	
//		maxd.open("BMCP_maxdepth",std::ios_base::app);
		
		rolloutCountDisp = new uint*[Simulator.getSizeX()];
		for(uint xx=0;xx<Simulator.getSizeX();++xx){
			rolloutCountDisp[xx] = new uint[Simulator.getSizeY()];
			std::fill(rolloutCountDisp[xx],rolloutCountDisp[xx]+Simulator.getSizeY(),0);
		}
		
		drawn = new uint*[Simulator.getSizeX()*2];
		for(uint xx=0;xx<2*Simulator.getSizeX();++xx){
			drawn[xx] = new uint[Simulator.getSizeY()*2];
			std::fill(drawn[xx],drawn[xx]+Simulator.getSizeY()*2,0);	
		}
		state2 = new State2D(NumC);
		rolloutHistory.reserve(Params.MaxDepth);

		//Initialize Q value
		X=Simulator.getSizeX();
		Y=Simulator.getSizeY();
		Q = new double**[X*Y*MAXPREDWAIT];
		QU = new uint*[X*Y*MAXPREDWAIT];//Update counts
		for(uint pxy = 0; pxy < X*Y*MAXPREDWAIT; ++pxy){
			Q[pxy] = new double*[X*Y];
			QU[pxy] = new uint[X*Y];
			std::fill(QU[pxy],QU[pxy]+X*Y,0);
			for(uint xy = 0; xy < X*Y; ++xy){
				Q[pxy][xy] = new double[A];
				std::fill(Q[pxy][xy],Q[pxy][xy]+A,0.0);
			}
		}
		
		XYvisited = new bool*[X];
		for(uint xx=0;xx<X;++xx){
			XYvisited[xx] = new bool[Y];
			std::fill(XYvisited[xx],XYvisited[xx]+Y,false);
		}
		
		QP = new double[A];

		
}

BAMCP2D::~BAMCP2D()
{
	VNODE2D::Free(Root);
	VNODE2D::FreeAll();
//	meand << std::endl;
//	meand.close();
//	maxd << std::endl;
//	maxd.close();
	
		for(uint xx=0;xx<Simulator.getSizeX();++xx){
			delete[] rolloutCountDisp[xx];
		}
		for(uint xx=0;xx<2*Simulator.getSizeX();++xx){
			delete[] drawn[xx];
		}

		delete[] rolloutCountDisp;
		delete[] drawn;
		delete state2;
		
		for(uint pxy = 0; pxy < X*Y*MAXPREDWAIT; ++pxy){
			for(uint xy = 0; xy < X*Y; ++xy){
				delete[] Q[pxy][xy];
			}
			delete[] Q[pxy];
			delete[] QU[pxy];
		}
		delete[] Q;
		delete[] QU;
		delete[] QP;

		for(uint xx=0;xx<X;++xx)
			delete[] XYvisited[xx];
		delete[] XYvisited;
		
}

bool BAMCP2D::Update(State2D* ss, uint aa, State2D* observation, double reward)
{
    //History.Add(aa, observation);
	
		//Update posterior
		//std::cout << "(" << ss->x << "," << ss->y << ") " << aa << " (" << observation->x << "," << observation->y << ") " << std::endl;
		std::cout << "|" << std::flush;
		SampFact.update(ss,aa,observation);	
		sumRewards += reward;

		//Reuse previous subtree for next search 
		//Get to subtree
    QNODE2D& qnode = Root->Child(aa);
		VNODE2D* vnode = qnode.Child(observation);
		if(vnode && Params.ReuseTree && step > 2){
			//std::cout << VNODE2D::VNodePool.GetNumAllocated() << std::endl;
			//Delete other subtrees but this one
			vnode->preventfree = true;
			VNODE2D::Free(Root);
			vnode->preventfree = false;
			Root = vnode;
		}else{ 
			// Delete old tree and create new root
			VNODE2D::Free(Root);
			VNODE2D* newRoot = ExpandNode(observation); //ss);
			Root = newRoot;
		}	
    return true;
}

int BAMCP2D::SelectAction(State2D* current_state)
{
    UCTSearch(current_state);
	
		step++;

		return GreedyUCB(Root, false);
}

void BAMCP2D::UCTSearch(State2D* state)
{
		//Clear rollout counts (DISPLAY)
		if(display){
			for(uint xx=0;xx<Simulator.getSizeX();++xx){
				std::fill(rolloutCountDisp[xx],rolloutCountDisp[xx]+Simulator.getSizeY(),0);
			}
		}
		maxRolloutVisits = 0;
		//------------------------------
    ClearStatistics();
		//int historyDepth = History.Size();
		Sampler2D* MDPSampler = 0;
		MDPSampler = SampFact.getMDPSampler(Simulator.GetDiscount());
		
    boost::timer timer;
		
    for (int n = 0; n < Params.NumSimulations; n++)
		{
			if (Params.Timeout > 0 && timer.elapsed() > Params.Timeout){
				//std::cout << "Effective number of simulations: " << n << std::endl;
				break;
			}
			// Sample MDP given counts	
			mdp = MDPSampler->updateMDPSample();

			if (Params.Verbose >= 2)
			{
				cout << "Starting simulation" << endl;
				//Simulator.DisplayState(*state, cout);
			}

			TreeDepth = 0;
			PeakTreeDepth = 0;

			double totalReward = SimulateV(mdp, state, Root);
			StatTotalReward.Add(totalReward);
			StatTreeDepth.Add(PeakTreeDepth);

			if (Params.Verbose >= 2)
				cout << "Total reward = " << totalReward << endl;
			//if (Params.Verbose >= 4)
			//DisplayValue(4, cout);

			//History.Truncate(historyDepth);

			if(display && (n % std::max(1,Params.NumSimulations/5)) == 0)
				this->Display(state,Params.NumSimulations-n);

		}
		delete MDPSampler;
		DisplayStatistics(cout);
		//meand << StatTreeDepth.GetMean() << " ";
		//maxd << StatTreeDepth.GetMax() << " ";
}

double BAMCP2D::SimulateV(SIMULATOR2D* mdp, State2D* state, VNODE2D* vnode)
{
	uint action = GreedyUCB(vnode, true);

	PeakTreeDepth = TreeDepth;
	if (TreeDepth >= Params.MaxDepth) // search horizon reached
		return 0;

  QNODE2D& qnode = vnode->Child(action);
  double totalReward = SimulateQ(mdp, state, qnode, action);
  vnode->Value.Add(totalReward);

  return totalReward;
}

double BAMCP2D::SimulateQ(SIMULATOR2D* mdp, State2D* state, QNODE2D& qnode, uint action)
{
    State2D* observation = new State2D(NumC); //FIXME Use memory pool
    double immediateReward, delayedReward = 0;

    bool terminal = mdp->Step(state, action, observation, immediateReward);
	    
		VNODE2D*& vnode = qnode.Child(observation);
    if (!vnode && !terminal && qnode.Value.GetCount() >= Params.ExpandCount)
        vnode = ExpandNode(observation);

    if (!terminal)
    {
        TreeDepth++;
        if (vnode)
            delayedReward = SimulateV(mdp, observation, vnode);
        else{
					delayedReward = Rollout(mdp, observation);
				}
        TreeDepth--;
    }
		
    double totalReward = immediateReward + mdp->GetDiscount() * delayedReward;
    qnode.Value.Add(totalReward);
		delete observation;

    return totalReward;
}

VNODE2D* BAMCP2D::ExpandNode(State2D* state)
{
    VNODE2D* vnode = VNODE2D::Create(state->x,state->y);
    vnode->Value.Set(0, 0);
		vnode->SetChildren(0,0);

    return vnode;
}


int BAMCP2D::GreedyUCB(VNODE2D* vnode, bool ucb) const
{
    static std::vector<int> besta;
    besta.clear();
    double bestq = -Infinity;
    int N = vnode->Value.GetCount();
    double logN = log(N + 1);

    for (uint action = 0; action < Simulator.GetNumActions(); action++)
    {
        double q;
        int n;

        QNODE2D& qnode = vnode->Child(action);
        q = qnode.Value.GetValue();
        n = qnode.Value.GetCount();

				if (ucb)
            q += FastUCB(N, n, logN);
				
        if (q >= bestq)
        {
            if (q > bestq)
                besta.clear();
            bestq = q;
            besta.push_back(action);
        }
    }

    assert(!besta.empty());
    return besta[Random(besta.size())];
}

double BAMCP2D::Rollout(SIMULATOR2D* mdp, State2D* state)
{

    if (Params.Verbose >= 3)
        cout << "Starting rollout" << endl;

    double totalReward = 0.0;
    double discount = 1.0;
    bool terminal = false;
    int numSteps;
		int action = -10;
		int last_action;
	
		rolloutHistory.clear();
		State2D* observation = state2;
		State2D* tmp;
    for (numSteps = 0; numSteps + TreeDepth < Params.MaxDepth && !terminal; ++numSteps)
		{

			last_action = action;
			double reward;

			if(Params.RolloutMode == 2){ // No backtracking of rollouts
				last_action = action;
				action = Random(A);
				while(abs(last_action-action)==2)
					action = Random(A);
			}
			else
				action = Random(A);

      terminal = mdp->Step(state, action, observation, reward);

      rolloutHistory.push_back(std::pair<uint,uint>(observation->x,observation->y));
      tmp = state;
      state = observation;
      observation = tmp;
      totalReward += reward * discount;
      discount *= mdp->GetDiscount();
    }

    StatRolloutDepth.Add(numSteps);
    if (Params.Verbose >= 3)
        cout << "Ending rollout after " << numSteps
            << " steps, with total reward " << totalReward << endl;

    return totalReward;
}


double BAMCP2D::UCB[UCB_N][UCB_n];
bool BAMCP2D::InitialisedFastUCB = true;

void BAMCP2D::InitFastUCB(double exploration)
{
    cout << "Initialising fast UCB table with exp. const. " 
			<< exploration << "...  ";
    for (int N = 0; N < UCB_N; ++N)
        for (int n = 0; n < UCB_n; ++n)
            if (n == 0)
                UCB[N][n] = Infinity;
            else
                UCB[N][n] = exploration * sqrt(log(N + 1) / n);
    cout << "done" << endl;
    InitialisedFastUCB = true;
}


inline double BAMCP2D::FastUCB(int N, int n, double logN) const
{
    if (InitialisedFastUCB && N < UCB_N && n < UCB_n)
        return UCB[N][n];

    if (n == 0)
        return Infinity;
    else
        return Params.ExplorationConstant * sqrt(logN / n);
}

void BAMCP2D::ClearStatistics()
{
    StatTreeDepth.Clear();
    StatRolloutDepth.Clear();
    StatTotalReward.Clear();
}

void BAMCP2D::DisplayStatistics(ostream& ostr) const
{
    if (Params.Verbose >= 1)
    {
        StatTreeDepth.Print("Tree depth", ostr);
        StatRolloutDepth.Print("Rollout depth", ostr);
        StatTotalReward.Print("Total reward", ostr);
    }


}


//Viz tree display --------------------------------------------------------
void BAMCP2D::Display(State2D* currentState, uint simLeft){
#if defined(GX)
	Simulator.renderMAPXYOffset(moffsetx,moffsety);
	display->StartRender();	
	display->drawAgent(Root->x+moffsetx,Root->y+moffsety);

	if(currentState->wait >= 0){
		display->drawPredator(currentState->x2,currentState->y2);
	}

	//Display tree search -----------------------
	glColor3f(0,0.6,0);
	if(Root && Params.DisplayMode > 0){
		//for(uint xx=0;xx<2*Simulator.getSizeX();++xx){
	//		std::fill(drawn[xx],drawn[xx]+2*Simulator.getSizeY(),0);	
	//	}
		//if(Params.DisplayMode > 1){
			DisplayTreeV(Root,0);
		//}else{
		//	QNODE2D& qnode = Root->Child(2);	
	  //		DisplayTreeQ(qnode,Root);
		//}
	}
	//===========================================
	
	//Display Rollout "heat" map ----------------
	double sx1,sy1;
	double offset;
/*
	display->getScreenStartPosition2(sx1,sy1,offset);
	double visits;
	glPushMatrix();
	glTranslated(sx1-offset/2,sy1-offset/2,0.0);
	for(uint xx=0;xx<Simulator.getSizeX();++xx){
		glPushMatrix();
		for(uint yy=0;yy<Simulator.getSizeY();++yy){
			visits = rolloutCountDisp[xx][yy];
			if(visits > 0)
				glColor3f(std::max(0.1,visits/(maxRolloutVisits/2.0)),0.0,0.0);
			else
				glColor3f(0.0,0.0,0.0);
			glBegin(GL_QUADS);
				glVertex3f(0.0f, offset, 0.0f);
				glVertex3f(offset,offset, 0.0f);
				glVertex3f(offset,0.0f, 0.0f);
				glVertex3f(0.0f,0.0f, 0.0f);
			glEnd();				
			glTranslated(0.0,offset,0.0);
		}
		glPopMatrix();
		glTranslated(offset,0.0,0.0);	
	}
	glPopMatrix();
	//===========================================
*/	

	int ssize;
	
	//Display observations ----------------------
	//display->getScreenStartPosition3(sx1,sy1,offset);
	display->getScreenStartPosition2(sx1,sy1,offset);
	sy1=sy1-18;
	sx1=sx1-10;
	SampFact.render(sx1,sy1,offset);
	
	ssize = sprintf(dispstr,"Observed Transitions", step);
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0,1.0,1.0);
	glPushMatrix();
	glTranslated(sx1+5,sy1+5,0.0);
		display->font->drawString(dispstr,ssize);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	//-------------------------------------------

	//Display current sample --------------------
	display->getScreenStartPosition4(sx1,sy1,offset);
	mdp->render(sx1,sy1,offset);	
	//-------------------------------------------
	ssize = sprintf(dispstr,"Lazy Samples", step);
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0,1.0,1.0);
	glPushMatrix();
	glTranslated(sx1+5,sy1+5,0.0);
		display->font->drawString(dispstr,ssize);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);

	/*
	//Display last rollout ----------------------
	glPushMatrix();
	glTranslated(sx1,sy1,0.0);
	glLineWidth(1.0);
	glColor3f(0.9,0.7,0.7);
	double correction = offset/2;//for centering on square
	double xx = rolloutHistory[0].first+1;
	double yy = rolloutHistory[0].second+1;
	double xx2,yy2;
	for(uint ii=0;ii<rolloutHistory.size();++ii){
		xx2 = rolloutHistory[ii].first+1;
		yy2 = rolloutHistory[ii].second+1;
		glBegin(GL_LINES);
			glVertex3f(xx*offset+correction,yy*offset+correction,0.0f);
			correction = offset/2 - 2 + utils::rng.rand_closed01()*4; //Comment for straight lines
			glVertex3f(xx2*offset+correction,yy2*offset+correction,0.0f);
		glEnd();
		xx = xx2;
		yy = yy2;
	}
	glPopMatrix();
	//-------------------------------------------
*/	
	//Display Info ------------------------------
	ssize = sprintf(dispstr,"Step: %d", step);
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0,1.0,1.0);
	glPushMatrix();
	glTranslated(35.0,2.0,0.0);
	display->font->drawString(dispstr,ssize);
	ssize = sprintf(dispstr,"Sims left: %d", simLeft);
	glTranslated(75.0,0.0,0.0);
	display->font->drawString(dispstr,ssize);
	glTranslated(135.0,0.0,0.0);
	ssize = sprintf(dispstr,"Sum rewards: %.1f", sumRewards);
	display->font->drawString(dispstr,ssize);
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
	//===========================================
	display->StopRender();

#endif
}
void BAMCP2D::DisplayTreeV(VNODE2D* vnode, uint depth){
	if(vnode){
		for(uint aa = 0; aa < A; ++aa){
			 QNODE2D& qnode = vnode->Child(aa);	
			 DisplayTreeQ(qnode,vnode, depth);
		}
	}
}
void BAMCP2D::DisplayTreeQ(QNODE2D& qnode, VNODE2D* parent, uint depth){
	if(qnode.childrenP[0] != 0)
		DisplayTreeQAT(qnode.childrenP[0],parent, depth);
	if(qnode.childrenP[1] != 0)
		DisplayTreeQAT(qnode.childrenP[1],parent, depth);
}

void BAMCP2D::DisplayTreeQAT(QNODE2DAT* qnodeat, VNODE2D* parent,uint depth){
#if defined(GX)
	if(NumC > 1 && qnodeat->level != NumC-1){ 
		for(uint ii=0;ii<NumAT;++ii){
			if(qnodeat->childrenAT[ii] != 0){
				DisplayTreeQAT(qnodeat->childrenAT[ii], parent, depth);
			}
		}
	}
	else{
		for(uint ii=0;ii<NumAT;++ii){
			if(qnodeat->childrenV[ii] != 0){
				uint tempx = qnodeat->childrenV[ii]->x;
				uint tempy = qnodeat->childrenV[ii]->y;
			//	if(drawn[tempx+parent->x][tempy+parent->y] < qnodeat->childrenV[ii]->Value.GetCount()){
					double sx1,sy1,sx2,sy2;
					display->getScreenPosition(parent->x+moffsetx,parent->y+moffsety,sx1,sy1);
					display->getScreenPosition(tempx+moffsetx,tempy+moffsety,sx2,sy2);
					double count = qnodeat->childrenV[ii]->Value.GetCount();
					double value = qnodeat->childrenV[ii]->Value.GetValue();
					glLineWidth(std::max(1.0,count/10));
					if(value > 0)
						glColor3f(0.0,value*10,0.0);
					else	
						glColor3f(-value/10.0,0.0,0.0);
					//std::cout << value << " " << depth << std::endl;	
					glBegin(GL_LINES);
					glVertex3f(sx1,sy1,0.0f);
					glVertex3f(sx2,sy2,0.0f);
					glEnd();
			//		if(Params.DisplayMode > 1)
			//			drawn[tempx+parent->x][tempy+parent->y] = qnodeat->childrenV[ii]->Value.GetCount();
			//	}
				DisplayTreeV(qnodeat->childrenV[ii],depth+1);
			}
		}
		
	}		
#endif
}
//END Viz tree display --------------------------------------------------------


//-----------------------------------------------------------------------------
