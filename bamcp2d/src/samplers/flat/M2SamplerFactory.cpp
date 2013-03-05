#include "M2SamplerFactory.h"

#include "M2Sampler.h"



M2SamplerFactory::M2SamplerFactory(){
	RLocation = 6;
}

Sampler2D* M2SamplerFactory::getMDPSampler(double gamma){
			
	M2Sampler* sampler = new M2Sampler(gamma, RLocation);

	return (Sampler2D*) sampler;
}

SamplerFactory2D* M2SamplerFactory::createNewClone(){
	return (SamplerFactory2D*) new M2SamplerFactory();
}

uint getSensorAT(int xdiff, int ydiff, uint* AT){
	uint sensorAT = 0;
	if(xdiff==1 && ydiff == 1)
		sensorAT = AT[0];	
	else if(xdiff==1 && ydiff == 0)
		sensorAT = AT[1];
	else if(xdiff==1 && ydiff == -1)
		sensorAT = AT[2];
	else if(xdiff==0 && ydiff == -1)
		sensorAT = AT[3];
	else if(xdiff==0 && ydiff == 0)
		sensorAT = AT[4];
	else if(xdiff==0 && ydiff == 1)
		sensorAT = AT[5];
	else if(xdiff==-1 && ydiff == 1)
		sensorAT = AT[6];
	else if(xdiff==-1 && ydiff == 0)
		sensorAT = AT[7];
	else if(xdiff==-1 && ydiff == -1)
		sensorAT = AT[8];
	else
		assert(0);
	return sensorAT;
}

void M2SamplerFactory::Clone(SamplerFactory2D* parent){
	RLocation = ((M2SamplerFactory*)parent)->RLocation;
}

void M2SamplerFactory::update(State2D* state, uint a, State2D* obs){
	//Only observations that has an effect on posterior
	
	if(abs(obs->x - XM2S) < 2 && abs(obs->y - YM2S) < 2){
		//std::cout << "SENSOR OBSERVED" << std::endl;
		int xdiff = obs->x-XM2S;
		int ydiff = obs->y-YM2S;
		uint sensorAT = getSensorAT(xdiff,ydiff,obs->AT);
		if(sensorAT == 3){
			RLocation = 0;	
		}
		else if(sensorAT == 5){
			RLocation = 2;	
		}
		else{ // == 4
			RLocation = 1;
		}
	}

	if(abs(obs->x - XM2LR) < 2 && abs(obs->y- YM2R) < 2){
		
		int xdiff = obs->x-XM2LR;
		int ydiff = obs->y-YM2R;
		//std::cout << "Left Reward Location Observed" << std::endl;
		uint sensorAT = getSensorAT(xdiff,ydiff,obs->AT);
		if(sensorAT == 1){ //Reward location directly observed
			RLocation = 0;
		}
		else{
			if(RLocation == 6) //NO obs
				RLocation = 5;
			else if(RLocation == 3)
				RLocation = 1;
			else if(RLocation == 4)
				RLocation = 2;
		}
		
	}

	if(abs(obs->x - XM2RR) < 2 && abs(obs->y- YM2R) < 2){
		int xdiff = obs->x-XM2RR;
		int ydiff = obs->y-YM2R;
		//std::cout << "Right Reward Location Observed" << std::endl;
		uint sensorAT = getSensorAT(xdiff,ydiff,obs->AT);
		if(sensorAT == 1){ //Reward location directly observed
			RLocation = 2;	
		}
		else{
			if(RLocation == 6) //NO obs
				RLocation = 3;
			else if(RLocation == 4)
				RLocation = 0;
			else if(RLocation == 5)
				RLocation = 1;
		}
	}

	if(abs(obs->x - XM2TR) < 2 && abs(obs->y- YM2TR) < 2){
		int xdiff = obs->x-XM2TR;
		int ydiff = obs->y-YM2TR;
		//std::cout << "Top Reward Location Observed" << std::endl;
		uint sensorAT = getSensorAT(xdiff,ydiff,obs->AT);
		if(sensorAT == 1){ //Reward location directly observed
			RLocation = 1;	
		}
		else{
			if(RLocation == 6) //NO obs
				RLocation = 4;
			else if(RLocation == 5)
				RLocation = 2;
			else if(RLocation == 3)
				RLocation = 0;
		}
	}


}

