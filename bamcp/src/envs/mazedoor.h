#pragma once

#include "simulator.h"

class MazeDoor : public SIMULATOR
{
public:

    MazeDoor(bool _open, double discount);
		~MazeDoor();

    virtual void Validate(const uint state) const;
    virtual uint CreateStartState() const;
    virtual bool Step(uint state, uint action, 
        uint& observation, double& reward) const;
        
    //void GeneratePreferred(const uint state,
     //   std::vector<int>& legal, const STATUS& status) const;
		void setOpen();
		void setClosed();

    virtual void DisplayState(const uint state, std::ostream& ostr) const;
    virtual void DisplayObservation(const uint state, uint observation, std::ostream& ostr) const;
    virtual void DisplayAction(int action, std::ostream& ostr) const;
		
protected:
		uint S,A;
		uint SA;
		uint L;
		bool open;
		double *Tc;
private:

};

