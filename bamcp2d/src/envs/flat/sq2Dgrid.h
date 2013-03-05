#pragma once

#include "simulator2D.h"

class SqGrid : public SIMULATOR2D
{
public:

    SqGrid(uint _x, uint _y, uint _numAttributes, double discount);
		~SqGrid();

		void Save(std::string filename);
		void Load(std::string filename);
   
		virtual bool move(State2D* state, uint action) const;
		
		virtual bool Step(State2D* state, uint action, 
        State2D* observation, double& reward); //const
        
		//virtual double getReward(State2D* ss, uint action, State2D* sp) const;

		//void setATmap(uint* _ATmap); careful with dimensions+wall
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

		virtual uint getRLPI(uint**& RLPI, State2D* state, double Timeout) const;
		virtual void deleteRLPI(uint**& RLPI) const;

		bool checkLegalStart();

protected:
		uint X,Y;
		uint A;
		double* ATr; //maps AT --> reward
		double moveCost;
		double PREDCOST; //Predator cost (of being close)
		int PREDWAIT; //Wait time between moves (-1 for NO PREDATOR)
	
		uint STARTX, STARTY; //agent's starting position
		uint STARTXP, STARTYP; //Predator's starting position

		bool* ATw; //maps AT --> wall? (only form of obstacle)
		uint* ATmap;
		
		double** ATcolor;
		void setATObs(State2D* observation, uint coord) const;

		//For computing policy
		void StepShort(uint pxy, uint xy, uint action, double& reward, uint& pxy2, uint& xy2) const;
		void solveVI(uint**&, double Timeout) const;

private:
	
		
};

