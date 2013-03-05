#include "maze.h"
#include "../utils/utils.h"
#include <algorithm>

using namespace UTILS;

Maze::Maze(double discount)
{
		A = 4;
		S = 264;
		NumObservations = S;
		NumActions = A;
		Discount = discount;
		rsas = false;

    RewardRange = 3;
			
		Tc = new double[2];
		Tc[1] = 0.9;
		Tc[0] = 0.1;

		//Standard description
		T = 0;
		
		//Number of free locations for each column
		FreeY = new uint[7];
		FreeY[0] = 5;
		FreeY[1] = 3;
		FreeY[2] = 6;
		FreeY[3] = 6;
		FreeY[4] = 4;
		FreeY[5] = 5;
		FreeY[6] = 4;
		
		grid.resize(7);
		grid[0] = {1, 1, 1, 0, 1, 1};
		grid[1] = {0, 0, 1, 0, 1, 1};
		grid[2] = {1, 1, 1, 1, 1, 1};
		grid[3] = {1, 1, 1, 1, 1, 1};
		grid[4] = {0, 0, 1, 1, 1, 1};
		grid[5] = {1, 1, 1, 0, 1, 1};
		grid[6] = {1, 1, 1, 0, 1, 0};

		R = new double[S*A];
		std::fill(R,R+S*A,0);
		
		//Reward non-zero when at goal
		//for each flag situation
		uint s;
		for(uint flag=0;flag<8;++flag){
			s = compress(flag,6,0);
			//Number of flags
			int r = __builtin_popcount(flag);
			R[s*A] = r;
			R[s*A+1] = r;
			R[s*A+2] = r;
			R[s*A+3] = r;
		}
		
		//For displaying
		gridStr.push_back(" 0 . X $ . X . G ");
		gridStr.push_back(" 1 . X . . X . . ");    
		gridStr.push_back(" 2 . . . . . . . ");    
		gridStr.push_back(" 3 X X . . . X X ");
		gridStr.push_back(" 4 . . . . . . $ ");   
		gridStr.push_back(" 5 $ . . . . . X ");
		gridStr.push_back("   0 1 2 3 4 5 6 ");


		
}                                     
/*                                   
 * Maze domain
 *  $ - Flags
 *  G - goal
 *  X - walls
 *
 *
 *  0 . X $ . X . G      
 *  1 . X . . X . .    
 *  2 . . . . . . .        
 *  3 X X . . . X X             
 *  4 . . . . . . $        
 *  5 $ . . . . . X
 *    0 1 2 3 4 5 6 
 *
 */
bool Maze::validXY(uint x, uint y) const{
	if(x==0 && y == 3)
		return false;
	if(x==1 && (y == 3 || y < 2))
		return false;
	if(x==4 && y < 2)
		return false;
	if(y==3 && x>4)
		return false;
	if(x==6 && y == 5)
		return false;

	if(x >= 0 && x < 7 && y >= 0 && y < 6)
		return true;
	else
		return false;
}

uint Maze::compress(uint flag, uint x, uint y) const{		
	uint l = 0;
	for(uint i=0;i<x;++i){
		l += FreeY[i];
	}
	for(uint i=0;i<y;++i){
		l += grid[x][i];
	}
	uint state = l*8+flag; //8 possible flag
	return state;
}
void Maze::uncompress(uint state, uint& flag, uint& x, uint& y) const{
	uint l = state/8;
	flag = state - l*8;
	uint sum = 0;
	for(uint i=0;i<7;++i){
		for(uint j=0;j<6;++j){
			if(sum == l && grid[i][j] != 0){
				x = i;
				y = j;
				return;
			}
			sum += grid[i][j];
		}
	}
	x=6;
	y=4;
}


Maze::~Maze(){
	if(R != 0)
		delete[] R;
	if(T != 0)
		delete[] T;
	delete[] Tc;
	delete[] FreeY;
}

uint Maze::CreateStartState() const
{ 
		return 0; //(rand() % S);
}


void Maze::Validate(const uint state) const{

}

bool Maze::Step(uint state, uint action, 
    uint& observation, double& reward) const
{
	uint success;
	reward = R[state*A+action];
	utils::rng.multinom(Tc,2,success);

	//Action mapping
	//       0
	//       ^
	//       |
	//  3 <-- --> 1
	//       |
	//       v
	//       2
	uint x,y,flag;
	uncompress(state, flag, x, y);
	uint nextx;
	uint nexty;

	if(x==6 && y==0){ //Goal
		nextx = 0;
		nexty = 0;
		flag = 0;
	}else{

		//slip
		if(!success){
			if(utils::rng.rand_closed01() < 0.5)
				action = (action + 1) % A;
			else
				action = (action - 1) % A;
		}

		nextx = x;
		nexty = y;
		if(action == 0)
			nexty -= 1;
		else if(action == 2)
			nexty += 1;
		else if(action == 3)
			nextx -= 1;
		else if(action == 1)
			nextx += 1;

		//if collision, stay where you are
		if(!validXY(nextx,nexty)){
			nextx = x;
			nexty = y;
		}
		
		
		//Update flag
		if((nextx == 0 && nexty == 5))
			flag = flag | 1;
		else if(nextx == 2 && nexty == 0)
			flag = flag | 2;
		else if(nextx == 6 && nexty == 4)
			flag = flag | 4;
		
	}

	observation = compress(flag,nextx,nexty);
	//std::cout << "(" << action << ":" << reward << ":" << __builtin_popcount(flag) << "," << nextx << "," << nexty << ") ";
	//std::cout << state << " " << observation << std::endl;
	//DisplayState(state,std::cout);
	//DisplayAction(action,std::cout);
	//std::cout << std::endl;
	return false;
}


void Maze::DisplayState(const uint state, std::ostream& ostr) const{	
	uint x,y,flag;
	uncompress(state,flag,x,y);
	for(uint i=0;i<gridStr.size();++i){
		if(i!=y)
			std::cout << gridStr[i] << std::endl;
		else{
			std::string tmp = gridStr[i];
			tmp[2*x+3] = 'A';
			std::cout << tmp << std::endl;
		}
	}
	std::cout << "Flag status: " << ((flag & 1) > 0) << " " << ((flag & 2) > 0) << " " << ((flag & 4) > 0) << "     ";
}

void Maze::DisplayObservation(const uint state, uint observation, std::ostream& ostr) const {}

void Maze::DisplayAction(int action, std::ostream& ostr) const {
        ostr << action << std::endl;
}
