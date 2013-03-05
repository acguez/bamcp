#pragma once

#include "simulator.h"

class Dloop : public SIMULATOR
{
public:

    Dloop(double discount);
		~Dloop();

    virtual void Validate(const uint state) const;
    virtual uint CreateStartState() const;
    virtual bool Step(uint state, uint action, 
        uint& observation, double& reward) const;
        
    virtual void DisplayState(const uint state, std::ostream& ostr) const;
    virtual void DisplayObservation(const uint state, uint observation, std::ostream& ostr) const;
    virtual void DisplayAction(int action, std::ostream& ostr) const;
		
protected:
		uint S,A;
};

