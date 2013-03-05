#pragma once

#include "simulator.h"

class Grid : public SIMULATOR
{
public:

    Grid(uint _S,double discount);
		~Grid();

    virtual void Validate(const uint state) const;
    virtual uint CreateStartState() const;
    virtual bool Step(uint state, uint action, 
        uint& observation, double& reward) const;
        
    virtual void DisplayState(const uint state, std::ostream& ostr) const;
    virtual void DisplayObservation(const uint state, uint observation, std::ostream& ostr) const;
    virtual void DisplayAction(int action, std::ostream& ostr) const;
		
protected:
		uint S,A;
		uint L;
		double *Tc;
private:

};

