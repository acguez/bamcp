#pragma once
#include "utils/utils.h"

//Bernoulli bandits and different algorithms to solve them
class Bandit{
	public:
		Bandit(){A = 0;}
		~Bandit();
		Bandit(uint _A, double _gamma, double* _p);
		
		uint step(uint arm);

		//Use Gittins indices to explore
		uint gittinsLearn(std::vector<uint>& Rhist, std::vector<uint>& optArm, uint N);
		//Use sboss to solve (transformed) bandit
		// Transform from 1 state MDP to 2 state (R=0,R=1) MDP with 
		// unknown transitions
	
		void sboss(std::vector<uint>& Rhist, std::vector<uint>& optArm,
		           size_t K, double delta, size_t N);
		void postMean(std::vector<uint>& Rhist, std::vector<uint>& optArm,
		          size_t N);



	private:
		double posteriorDeviation(double* counts1, double acounts2, double bcounts2);

		//Number of bandits
		uint A;
		//Discount factor
		double gamma;
	
		double** gittinsi;
		uint maxG;

		//Prob of success for each bandit
		double* p;
		//Best arm
		uint maxpi;
};
