#pragma once


class LazyInfGridSampler;

#include "simulator2D.h"

class LazyInfGrid : public SIMULATOR2D
{
public:

    LazyInfGrid(double discount, LazyInfGridSampler* _sampler);
		~LazyInfGrid();

		void Save(std::string filename);
		void Load(std::string filename);
   
		virtual bool move(State2D* state, uint action) const;
		
		virtual bool Step(State2D* state, uint action, 
        State2D* observation, double& reward);
        
		//virtual double getReward(State2D* ss, uint action, State2D* sp) const;

		//void setATmap(uint* _ATmap); careful with dimensions+wall
		//void setATxy(uint AT, uint x, uint y);
		virtual uint getATxy(uint x, uint y) const;

		virtual	void CreateStartState(State2D* state) const;

		virtual void render(double,double,double) const;
		virtual void renderMAPXYOffset(double& offsetx,double& offsety) const{ offsetx=0;offsety=0;}

		virtual uint getSizeX() const {return 0;}
		virtual uint getSizeY() const {return 0;}
	
		void setSTART(uint _sx, uint _sy){STARTX = _sx; STARTY = _sy;}

		virtual uint getRLPI(uint**& RLPI, State2D* state, double Timeout) const;
		virtual void deleteRLPI(uint**& RLPI) const;
	
		virtual void reset();

		bool checkLegalStart(){return true;}

		double** ATcolor;
protected:
		uint A;
		
		uint STARTX, STARTY; //agent's starting position

		uint boundX;
		uint boundY;
		
		LazyInfGridSampler* sampler;
		
};

