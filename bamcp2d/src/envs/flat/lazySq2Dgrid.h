#pragma once

#include "simulator2D.h"

class LazyMixtureSampler;

class LazySqGrid : public SIMULATOR2D
{
public:

    LazySqGrid(uint _x, uint _y, uint _numAttributes, double discount, LazyMixtureSampler* _samp);
		~LazySqGrid();

   
		virtual bool move(State2D* state, uint action) const;
		
		virtual bool Step(State2D* state, uint action, 
        State2D* observation, double& reward);
		double getReward(State2D* ss, uint action, State2D* sp) const {assert(0);}

		void setATxy(uint AT, uint x, uint y);
		virtual uint getATxy(uint x, uint y) const;

		virtual	void CreateStartState(State2D* state) const;

		inline uint getCoord(uint x, uint y) const;
		void getXY(uint coord, uint& x, uint& y) const;
		
		void setMoveCost(double cost);
		void setPREDVARS(double _PREDCOST, double _PREDWAIT);

		void setATr(uint AT, double r);

		virtual void render(double,double,double) const;
		virtual void renderMAPXYOffset(double& offsetx,double& offsety) const{ offsetx=0;offsety=0;}
		
		virtual uint getSizeX() const {return X;}
		virtual uint getSizeY() const {return Y;}

		void setSTART(uint _sx, uint _sy){STARTX = _sx; STARTY = _sy;}
		void setSTARTP(uint _sx, uint _sy){STARTXP = _sx; STARTYP = _sy;}

		double** ATcolor;

		virtual uint getRLPI(uint**& RLPI, State2D* state, double Timeout) const {return 0;}
		virtual void deleteRLPI(uint**& RLPI) const{}
		
protected:
		uint X,Y;
		uint A;
		double* ATr; //maps AT --> reward
		double moveCost;
		double PREDCOST; //Predator cost (of being close)
		double PREDWAIT; //Wait time between moves (-1 for NO PREDATOR)
		
		uint STARTX, STARTY;
		uint STARTXP, STARTYP;
		
		bool* ATw; //maps AT --> wall? (only form of obstacle)
		
		LazyMixtureSampler* sampler;
		
		void setATObs(State2D* observation, uint xx, uint yy) const;
private:
	
};

