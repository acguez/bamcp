#pragma once 

#include "utils/utils2.h"
#include <iostream>
#include "planners/mcp/value.h"

typedef unsigned int uint;

class SIMULATOR2D;
class State2D;
class QNODE2D;
class VNODE2D;


/* ------------------------
** Decision tree structure:
** ------------------------
**
**  VNODE
**  /   \
**   ... QNODE (position branching - )
**         \
**         QNODEAT0  (AT component 0 branching -- size NumAT)
**          \
**          QNODEAT1 (AT component 1 branching -- size NumAT)
**           .
**					 .
**					 .
**					 /
**			 QNODEAT($NUMC-2) (AT component ($NumC-2) branching -- size NumAT)
**			  /     \
**			VNODE	  VNODE  (AT component $NumC-1 branching -- size NumAT)
**
**	
**	Nodes below QNODE created on demand so tree should be sparse in most cases.
*/ 

//-----------------------------------------------------------------------------
class QNODE2DAT{
	public:
		~QNODE2DAT();
		void Initialise(uint _level);
		VNODE2D*& Child(State2D* state);
		//VNODE2D* Child(State2D* state) const;
		std::vector<QNODE2DAT*> childrenAT; 
		std::vector<VNODE2D*> childrenV;
		uint level;
	static uint NumC;
	static uint NumAT;
};


//-----------------------------------------------------------------------------

class QNODE2D
{
public:

    VALUE<int> Value;
		
		~QNODE2D();
		void Free(); // Memory pool frees  Vnode -> QNode (deletes internal tree TODO put in MemPool) -> frees child Vnode etc.
    void Initialise(uint x, uint y);

    VNODE2D*& Child(State2D* state);
    //VNODE2D* Child(State2D* state) const; 

    //static int NumC;
		//static int NumAT;

		QNODE2DAT* childrenP[2];  //size 2 for deterministic transition (wall or no wall)
private:
		uint x, y;

friend class VNODE2D;
};

//-----------------------------------------------------------------------------

class VNODE2D : public MEMORY_OBJECT
{
public:

    VALUE<int> Value;

    void Initialise(uint x, uint y);
    static VNODE2D* Create(uint x, uint y);
    static void Free(VNODE2D* vnode);
    static void FreeAll();

    QNODE2D& Child(int c) { return Children[c]; }
    const QNODE2D& Child(int c) const { return Children[c]; }

    void SetChildren(int count, double value);

    static uint NumChildren;
	
		bool preventfree;
		uint x, y;
private:
    std::vector<QNODE2D> Children;
    static MEMORY_POOL<VNODE2D> VNodePool;

};

