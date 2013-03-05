#pragma once

#include "simulator.h"
#include <string>

class Maze : public SIMULATOR
{
public:

    Maze(double discount);
		~Maze();

    virtual void Validate(const uint state) const;
    virtual uint CreateStartState() const;
    virtual bool Step(uint state, uint action, 
        uint& observation, double& reward) const;
        
//    void GeneratePreferred(const uint state,
//        std::vector<int>& legal, const STATUS& status) const;
//
		bool validXY(uint x, uint y) const;
		uint compress(uint flag, uint x, uint y) const;
		void uncompress(uint state, uint& flag, uint& x, uint& y) const;

    virtual void DisplayState(const uint state, std::ostream& ostr) const;
    virtual void DisplayObservation(const uint state, uint observation, std::ostream& ostr) const;
    virtual void DisplayAction(int action, std::ostream& ostr) const;
		
protected:
		uint S,A;
		uint *FreeY;
		double *Tc;

		std::vector<std::vector<uint> > grid;	
		std::vector<std::string> gridStr;
private:

};

