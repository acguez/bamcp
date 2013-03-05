#pragma once


#include "utils/utils2.h"
#include <map>
#include "envs/flat/simulator2D.h"

typedef unsigned int uint;
class QNODE3;
class VNODE3;

//-----------------------------------------------------------------------------
// Record state,reward,count associated with a VNODE3 
// (FIXME: move info to VNODE3? with unitialized bit)
struct VNODE3Pc : public MEMORY_OBJECT{
	public:
		VNODE3Pc(){state = 0;}
		~VNODE3Pc();
	
    void Initialise(State2D* _state);
		static VNODE3Pc* Create(State2D* _state);
		static void FreeAll();

		VNODE3* v;
		uint count;
		double reward;
		State2D* state;

		static int NumC;
	private:
			static MEMORY_POOL<VNODE3Pc> VNodePool;
};

//-----------------------------------------------------------------------------

class QNODE3
{
public:
		//Upper and lower bound on the value
    double Usa;
		double Lsa;
		uint Count;
		
		QNODE3(){Count=0;Children.reserve(ParamC);}
		void Add(State2D* state, double reward);
	
		std::vector<VNODE3Pc*> Children;
		
		VNODE3Pc* selectPath(double max,double min);

		static int ParamC;
};

//-----------------------------------------------------------------------------

class VNODE3 : public MEMORY_OBJECT
{
public:
		//Upper and lower bound on the value
		double Us;
		double Ls;

		uint bestA();
		
    void Initialise();
    static VNODE3* Create();
    void Free(VNODE3* vnode);
    static void FreeAll();

    QNODE3& Child(int c) { return Children[c]; }
    const QNODE3& Child(int c) const { return Children[c]; }

    void SetChildren(double max, double min);


    static int NumChildren;

		std::vector<QNODE3> Children;

private:
    static MEMORY_POOL<VNODE3> VNodePool;


};

