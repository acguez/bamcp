#include "experiment_baselines.h"
#include "boost/timer.hpp"

using namespace std;

EXPERIMENT_BASELINES::PARAMS::PARAMS()
:   NumSteps(300),
    TimeOut(15000),
		Mode(0),
		epsilon(0.2),
		alpha(0.2)
{
}

EXPERIMENT_BASELINES::EXPERIMENT_BASELINES(SIMULATOR2D& real,
    EXPERIMENT_BASELINES::PARAMS& expParams)
:   Real(real),
    ExpParams(expParams)
{
	RLPI = 0;
	Q=0;

	X = Real.getSizeX();
	Y = Real.getSizeY();
		
}

EXPERIMENT_BASELINES::~EXPERIMENT_BASELINES(){
	if(RLPI)
		Real.deleteRLPI(RLPI);
	if(Q){
		for(uint pxy = 0; pxy < X*Y*MAXPREDWAIT; ++pxy){
				for(uint xy = 0; xy < X*Y; ++xy){
					delete[] Q[pxy][xy];
				}
				delete[] Q[pxy];
		}
		delete[] Q;
	}
}
void EXPERIMENT_BASELINES::Run(std::vector<double>& Rhist)
{
		Real.reset();

		uint A = Real.GetNumActions();
	
		//Do some clean up
		if(RLPI)
			Real.deleteRLPI(RLPI);
		RLPI=0;

		if(Q){
			for(uint pxy = 0; pxy < X*Y*MAXPREDWAIT; ++pxy){
				for(uint xy = 0; xy < X*Y; ++xy)
					std::fill(Q[pxy][xy],Q[pxy][xy]+A,0);
			}
		}
		
		if(ExpParams.Mode == 2){
			//Initialize Q value
			Q = new double**[X*Y*MAXPREDWAIT];
			for(uint pxy = 0; pxy < X*Y*MAXPREDWAIT; ++pxy){
				Q[pxy] = new double*[X*Y];
				for(uint xy = 0; xy < X*Y; ++xy){
					Q[pxy][xy] = new double[A];
					std::fill(Q[pxy][xy],Q[pxy][xy]+A,0);
				}
			}
		}

    boost::timer timer;
   
		State2D* state = new State2D(Real.GetNumC());
		State2D* tmp;
		State2D* observation = new State2D(Real.GetNumC());
		
		Real.CreateStartState(state);
	

    double undiscountedReturn = 0.0;
    double discountedReturn = 0.0;
    double discount = 1.0;
    bool terminal = false;
    int t;

    for (t = 0; t < ExpParams.NumSteps; t++)
    {
				
        double reward;
				uint action;

				if(ExpParams.Mode == 0)
					action = rand() % A;	
				else if(ExpParams.Mode == 1)
					action = Real.getRLPI(RLPI,state,20.0);
				else{
					assert(state->wait < MAXPREDWAIT);
					if(utils::rng.rand_closed01() < ExpParams.epsilon)
						action = rand() % A;
					else{
						double bestq = -std::numeric_limits<double>::infinity(); 	
						uint besta=0;
						for(uint aa = 0; aa < A; ++aa){

							double q = Q[((state->x2+1)*Y+(state->y2+1))*MAXPREDWAIT+state->wait][(state->x+1)*Y+state->y+1][aa];
							if(q > bestq){
								bestq = q;
								besta = aa;
							}
						}
						action = besta;
					}
				}

        terminal = Real.Step(state, action, observation, reward);
				
				if(ExpParams.Mode == 2){ // QLearning update

					double bestq = -std::numeric_limits<double>::infinity(); 	
					for(uint aa = 0; aa < A; ++aa){
						double q = Q[((observation->x2+1)*Y+(observation->y2+1))*MAXPREDWAIT+observation->wait][(observation->x+1)*Y+observation->y+1][aa];
						if(q > bestq){
							bestq = q;
						}
					}
					uint pxyindex = ((state->x2+1)*Y+(state->y2+1))*MAXPREDWAIT+state->wait;
					uint xyindex = (state->x+1)*Y+state->y+1;
					Q[pxyindex][xyindex][action] = (1-ExpParams.alpha)*Q[pxyindex][xyindex][action]
						+ ExpParams.alpha*(reward + Real.GetDiscount()*bestq);
				}
				Rhist.push_back(reward);
				
        undiscountedReturn += reward;
        discountedReturn += reward * discount;
        discount *= Real.GetDiscount();

        if (terminal)
        {
            cout << "Terminated" << endl;
            break;
        }
				tmp = state;
				state = observation;
				observation = tmp;
				
				if (timer.elapsed() > ExpParams.TimeOut)
        {
            break;
        }
    }

    cout << "(" << discountedReturn << ":"
			<< undiscountedReturn 
			<< ") " << flush;

		delete state;
		delete observation;
}

//----------------------------------------------------------------------------
