#ifndef SIMULATOR2D_H
#define SIMULATOR2D_H

#include "utils/utils2.h"
#include <iostream>
#include <math.h>

typedef unsigned int uint;

//Not strictly state, other state components/information might be present in simulator state 
//based on history (e.g. see infinite grid where rewards are "consumed")
class State2D{
	public:
		State2D(uint n){AT=new uint[n];}
		uint x;
		uint y;
		uint* AT; //Attributes (other than location)
		uint x2; //Predator position
		uint y2;
		int wait;
		bool equals(State2D* state,uint n);
		void clone(State2D* state,uint n);
		~State2D(){delete[] AT;}
};

class SIMULATOR2D
{
public:

    SIMULATOR2D();
    SIMULATOR2D(uint numActions, uint numAttributes, uint numComponents, double discount = 1.0);    
    virtual ~SIMULATOR2D();

    // Create start state (can be stochastic)
    virtual void CreateStartState(State2D* state) const = 0;

		//Obstacle in the way?
		virtual bool move(State2D* state, uint action) const = 0;

    // Update state according to action, and get observation and reward. 
    // Return value of true indicates termination of episode (if episodic)
    virtual bool Step(State2D* state, uint action, 
        State2D* observation, double& reward) = 0;
		
		//virtual double getReward(State2D* ss, uint action, State2D* sp) const = 0;
    // Accessors
    uint GetNumActions() const { return NumActions; }
    uint GetNumAttributes() const { return NumAttributes; }
    uint GetNumC() const { return NumC; }
    bool IsEpisodic() const { return false; }
    double GetDiscount() const { return Discount; }
    double GetRewardRange() const { return RewardRange; }
    double GetHorizon(double accuracy, int undiscountedHorizon = 100) const;
	
		virtual uint getSizeX() const = 0;
		virtual uint getSizeY() const = 0;
	
		virtual void render(double,double,double) const = 0; //For displaying purposes
		virtual void renderMAPXYOffset(double&,double&) const = 0;

		virtual uint getATxy(uint x, uint y) const = 0;
		//Reward R(s,a,s') ? (otherwise reward function is R(s,a))
	//	bool rsas;a
	
		virtual uint getRLPI(uint**& RLPI, State2D* state, double Timeout) const = 0;
		virtual void deleteRLPI(uint**& RLPI) const = 0;

		virtual void reset(){};
protected:

    uint NumActions;
		uint NumAttributes; //Number of attributes for each location (other than position)
		uint NumC; //Number of attribute component contained in each state 
								// (e.g. 9 for square grid with +1 vision ...)

    double Discount, RewardRange;
};

#endif // SIMULATOR2D_H
