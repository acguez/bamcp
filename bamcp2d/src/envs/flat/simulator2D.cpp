#include "simulator2D.h"
#include <string.h>

using namespace std;
using namespace UTILS;


bool State2D::equals(State2D* state,uint n){
	if(x!=state->x)
		return false;
	if(y!=state->y)
		return false;
	if(x2!=state->x2)
		return false;
	if(y2!=state->y2)
		return false;
	if(wait != state->wait)
		return false;
	return !memcmp(AT,state->AT,n*sizeof(uint));	
}
void State2D::clone(State2D* state,uint n){
	x=state->x;
	y=state->y;
	x2=state->x2;
	y2=state->y2;
	wait=state->wait;
	memcpy(AT,state->AT,n*sizeof(uint));
}

SIMULATOR2D::SIMULATOR2D() 
:   
	NumActions(0),
	NumAttributes(0),
	NumC(0),
	Discount(1.0),
	RewardRange(1.0)
{
}

SIMULATOR2D::SIMULATOR2D(uint numActions, uint numAttributes, uint numComponents, double discount)
:   NumActions(numActions),
    NumAttributes(numAttributes),
		NumC(numComponents),
    Discount(discount)
{ 
    assert(discount > 0 && discount <= 1);
}

SIMULATOR2D::~SIMULATOR2D() 
{ 
}

double SIMULATOR2D::GetHorizon(double accuracy, int undiscountedHorizon) const 
{ 
    if (Discount == 1)
        return undiscountedHorizon;
    return log(accuracy) / log(Discount);
}
