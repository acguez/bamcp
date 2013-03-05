#pragma once

#include "simulator2D.h"

class InfGrid : public SIMULATOR2D
{
public:

    InfGrid(double discount,
				double* _beta_params_col,
				double* _beta_params_row);
		~InfGrid();

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
		virtual void renderMAPXYOffset(double& offsetx,double& offsety) const;

		virtual uint getSizeX() const {return 20;}
		virtual uint getSizeY() const {return 20;}
	
		void setSTART(uint _sx, uint _sy){STARTX = _sx; STARTY = _sy;}

		virtual uint getRLPI(uint**& RLPI, State2D* state, double Timeout) const;
		virtual void deleteRLPI(uint**& RLPI) const;
	
		virtual void reset();

		bool checkLegalStart(){return true;}

protected:

		double* beta_params_col;
		double* beta_params_row;

		uint boundX, boundY;
		uint A;
		
		uint STARTX, STARTY; //agent's starting position

		uint** ATmap;
		uint** map;
		double* prow; // p_i for each row
		double* betap_row; //Beta parameters for the row
		double* pcol; // p_j for each column
		double* betap_col; //Beta parameters for the column

		double** ATcolor;
	
		int lastx;
		int lasty;
		
		
};

