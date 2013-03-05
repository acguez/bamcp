 #include "node.h"
#include "utils/utils2.h"
#include <limits>


using namespace std;

//-----------------------------------------------------------------------------
MEMORY_POOL<VNODE3Pc> VNODE3Pc::VNodePool;

int VNODE3Pc::NumC = 0;

void VNODE3Pc::FreeAll()
{
	VNodePool.DeleteAll();
}
VNODE3Pc* VNODE3Pc::Create(State2D* _state){
	VNODE3Pc* vnode3pc = VNodePool.Allocate();
	vnode3pc->Initialise(_state);
	return vnode3pc;
}
void VNODE3Pc::Initialise(State2D* _state){
	if(state == 0)
			state = new State2D(VNODE3Pc::NumC);
	state->clone(_state,VNODE3Pc::NumC);
}
VNODE3Pc::~VNODE3Pc(){
	if(state != 0)
		delete state;
}
//-----------------------------------------------------------------------------
int QNODE3::ParamC = 0;
void QNODE3::Add(State2D* state, double reward){
	Count++;
	bool match=false;
	for(uint i=0;i<Children.size();++i){
		if(state->equals(Children[i]->state,VNODE3Pc::NumC)){
			Children[i]->count += 1;	
			match = true;
			break;
		}
	}
	if(!match){
		VNODE3Pc* node = VNODE3Pc::Create(state);
		node->v = 0; 
		node->count = 1;
		node->reward = reward;
		Children.push_back(node);
	}
}

VNODE3Pc* QNODE3::selectPath(double Vmax,double Vmin){	
	double bestDiff = 0;
	vector<uint> bestVNODE3;

	for(uint i=0; i < Children.size(); ++i){
		double diff;	
		if(Children[i]->v == 0){
			diff = (Vmax - Vmin)*Children[i]->count;	
		}else{
			diff = (Children[i]->v->Us-Children[i]->v->Ls)*Children[i]->count;
		}	
		if(diff > bestDiff){
			bestDiff = diff;
			bestVNODE3.clear();
			bestVNODE3.push_back(i);
		}
		else if(diff == bestDiff){
			bestVNODE3.push_back(i);
		}
	}
	return Children[bestVNODE3[rand() % bestVNODE3.size()]];
}
//-----------------------------------------------------------------------------

MEMORY_POOL<VNODE3> VNODE3::VNodePool;

int VNODE3::NumChildren = 0;

void VNODE3::Initialise()
{
	
    assert(NumChildren);
    Children.resize(VNODE3::NumChildren);
}

VNODE3* VNODE3::Create()
{
    VNODE3* vnode = VNodePool.Allocate();
    vnode->Initialise();
    return vnode;
}

void VNODE3::Free(VNODE3* vnode)
{
    for (int action = 0; action < VNODE3::NumChildren; action++){
			QNODE3& qnode = Children[action];
			for(uint i=0;i < qnode.Children.size(); i++){
        if (qnode.Children[i]->v)
					Free(qnode.Children[i]->v);
				}
		}
    VNodePool.Free(vnode);
}

void VNODE3::FreeAll()
{
	VNodePool.DeleteAll();
}

uint VNODE3::bestA(){
	vector<uint> besta;
	double bestq = -std::numeric_limits<double>::max();

	for(int aa=0;aa<NumChildren;++aa){
		QNODE3& qnode = Children[aa];
		double qa = qnode.Usa;
		if(qa > bestq){
			bestq = qa;
			besta.clear();
			besta.push_back(aa);
		}
		else if(qa == bestq)
			besta.push_back(aa);

		//std::cout << "Us," << aa << ": " << qa << std::endl;
	}
	
	return besta[rand() % besta.size()];
}

void VNODE3::SetChildren(double Usa, double Lsa)
{
    for (int action = 0; action < NumChildren; action++)
    {
        QNODE3& qnode = Children[action];
        qnode.Usa = Usa;
				qnode.Lsa = Lsa;
    }
}

//-----------------------------------------------------------------------------
