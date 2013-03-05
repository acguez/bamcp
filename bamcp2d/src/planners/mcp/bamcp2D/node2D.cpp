#include "node2D.h"
#include "utils/utils2.h"
#include "envs/flat/simulator2D.h"

using namespace std;

//-----------------------------------------------------------------------------
uint QNODE2DAT::NumC = 0;
uint QNODE2DAT::NumAT = 0;


QNODE2DAT::~QNODE2DAT(){
	if(NumC > 1 && level != (NumC-1)){ 
		for(uint at = 0; at < childrenAT.size(); at++){
			if(childrenAT[at] != 0)
				delete childrenAT[at];
		}
	}
	else{
		for(uint at = 0; at < childrenV.size(); at++){
			if(childrenV[at] != 0)
				VNODE2D::Free(childrenV[at]);
		}
	}
}
void QNODE2DAT::Initialise(uint _level){
	level = _level;

	if(NumC >  1 &&  level < (NumC-1)){ //Intermediary nodes - connect to other QNODE2DAT nodes 
		childrenAT.resize(NumAT);
		for(uint at = 0; at < NumAT; at++)
			childrenAT[at] = 0;
	}else{                         //Connect to VNODEs 
		childrenV.resize(NumAT);
		for(uint at = 0; at < NumAT; at++)
			childrenV[at] = 0;
	}
}

VNODE2D*& QNODE2DAT::Child(State2D* state){
	uint ii = state->AT[level];
	if(NumC > 1 && level != NumC-1){ 
		if(childrenAT[ii] == 0){
			childrenAT[ii] = new QNODE2DAT();
			childrenAT[ii]->Initialise(level+1);
		}
		return childrenAT[ii]->Child(state); 
	}
	else{
		return childrenV[ii]; //NO either return VNODE or return 0
	}
}
	//-----------------------------------------------------------------------------

void QNODE2D::Free(){
	if(childrenP[0] != 0){
		delete childrenP[0];
	}
	if(childrenP[1] != 0){
		delete childrenP[1];
	}
}
QNODE2D::~QNODE2D(){
	/*
	if(childrenP[0] != 0){
		delete childrenP[0];
	}
	if(childrenP[1] != 0){
		delete childrenP[1];
	}
	*/
}
void QNODE2D::Initialise(uint _x,uint _y)
{
	x = _x;
	y = _y;
	childrenP[0] = 0; //Stay at same position
	childrenP[1] = 0; //different position
}

//Traverse intermediate tree and create intermediate nodes on demand 
//have to know current VNODE position !
VNODE2D*& QNODE2D::Child(State2D* state){
	uint ii = 1;
	if(state->x == x && state->y == y)
		ii = 0;

	if(childrenP[ii] != 0)
		return childrenP[ii]->Child(state);
	else{
		childrenP[ii] = new QNODE2DAT();
		childrenP[ii]->Initialise(0);
		return childrenP[ii]->Child(state);
	}

}
//-----------------------------------------------------------------------------

MEMORY_POOL<VNODE2D> VNODE2D::VNodePool;

uint VNODE2D::NumChildren = 0;

void VNODE2D::Initialise(uint _x, uint _y)
{
		preventfree = false;
		x = _x;
		y = _y;
    assert(NumChildren);
    Children.resize(VNODE2D::NumChildren);
    for (uint action = 0; action < VNODE2D::NumChildren; action++)
        Children[action].Initialise(x,y);
}

VNODE2D* VNODE2D::Create(uint x, uint y)
{
    VNODE2D* vnode = VNodePool.Allocate();
    vnode->Initialise(x,y);
    return vnode;
}

void VNODE2D::Free(VNODE2D* vnode)
{
		if(!vnode->preventfree){
			VNodePool.Free(vnode);
			for (uint action = 0; action < VNODE2D::NumChildren; action++)
				vnode->Child(action).Free();
		}
}

void VNODE2D::FreeAll()
{
	VNodePool.DeleteAll();
}

void VNODE2D::SetChildren(int count, double value)
{
    for (uint action = 0; action < NumChildren; action++)
    {
        QNODE2D& qnode = Children[action];
        qnode.Value.Set(count, value);
    }
}
//-----------------------------------------------------------------------------
