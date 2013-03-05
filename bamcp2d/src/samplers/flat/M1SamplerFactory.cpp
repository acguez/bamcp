#include "M1SamplerFactory.h"

#include "M1Sampler.h"



M1SamplerFactory::M1SamplerFactory(){
	ObservedRLocation = false;
}

Sampler2D* M1SamplerFactory::getMDPSampler(double gamma){
			
	M1Sampler* sampler = new M1Sampler(gamma, ObservedRLocation, LeftR);

	return (Sampler2D*) sampler;
}

void M1SamplerFactory::Clone(SamplerFactory2D* parent){
	ObservedRLocation = ((M1SamplerFactory*)parent)->ObservedRLocation;
	LeftR = ((M1SamplerFactory*)parent)->LeftR;
}

SamplerFactory2D* M1SamplerFactory::createNewClone(){
	return (SamplerFactory2D*) new M1SamplerFactory();
}


void M1SamplerFactory::update(State2D* state, uint a, State2D* obs){
	//Only observations that has an effect on posterior
	
	uint sensorAT = 0; //"sensor" value
	if(abs(obs->x - XM1S) < 2 && abs(obs->y - YM1S) < 2){
		//std::cout << "SENSOR OBSERVED" << std::endl;
		int xdiff = obs->x-XM1S;
		int ydiff = obs->y-YM1S;
		//STUPID case by case (make this into a function x-y diff -> AT)
		if(xdiff==1 && ydiff == 1)
			sensorAT = obs->AT[0];	
		else if(xdiff==1 && ydiff == 0)
			sensorAT = obs->AT[1];
		else if(xdiff==1 && ydiff == -1)
			sensorAT = obs->AT[2];
		else if(xdiff==0 && ydiff == -1)
			sensorAT = obs->AT[3];
		else if(xdiff==0 && ydiff == 0)
			sensorAT = obs->AT[4];
		else if(xdiff==0 && ydiff == 1)
			sensorAT = obs->AT[5];
		else if(xdiff==-1 && ydiff == 1)
			sensorAT = obs->AT[6];
		else if(xdiff==-1 && ydiff == 0)
			sensorAT = obs->AT[7];
		else if(xdiff==-1 && ydiff == -1)
			sensorAT = obs->AT[8];

		ObservedRLocation = true;
		if(sensorAT == 3){
			LeftR = true;	
		}
		else{
			LeftR = false;	
		}
	}

	if(abs(obs->x - XM1LR) < 2 && abs(obs->y- YM1R) < 2){
		
		int xdiff = obs->x-XM1LR;
		int ydiff = obs->y-YM1R;
		//std::cout << "Reward Location OBSERVED" << std::endl;
		
		if(xdiff==1 && ydiff == 1)
			sensorAT = obs->AT[0];	
		else if(xdiff==1 && ydiff == 0)
			sensorAT = obs->AT[1];
		else if(xdiff==1 && ydiff == -1)
			sensorAT = obs->AT[2];
		else if(xdiff==0 && ydiff == -1)
			sensorAT = obs->AT[3];
		else if(xdiff==0 && ydiff == 0)
			sensorAT = obs->AT[4];
		else if(xdiff==0 && ydiff == 1)
			sensorAT = obs->AT[5];
		else if(xdiff==-1 && ydiff == 1)
			sensorAT = obs->AT[6];
		else if(xdiff==-1 && ydiff == 0)
			sensorAT = obs->AT[7];
		else if(xdiff==-1 && ydiff == -1)
			sensorAT = obs->AT[8];

		ObservedRLocation = true;
		if(sensorAT == 1){ //Reward location directly observed
			LeftR = true;	
		}
		else{
			LeftR = false;	
		}
	}

	if(abs(obs->x - XM1RR) < 2 && abs(obs->y- YM1R) < 2){
		int xdiff = obs->x-XM1RR;
		int ydiff = obs->y-YM1R;
		//std::cout << "Reward Location OBSERVED" << std::endl;
		
		if(xdiff==1 && ydiff == 1)
			sensorAT = obs->AT[0];	
		else if(xdiff==1 && ydiff == 0)
			sensorAT = obs->AT[1];
		else if(xdiff==1 && ydiff == -1)
			sensorAT = obs->AT[2];
		else if(xdiff==0 && ydiff == -1)
			sensorAT = obs->AT[3];
		else if(xdiff==0 && ydiff == 0)
			sensorAT = obs->AT[4];
		else if(xdiff==0 && ydiff == 1)
			sensorAT = obs->AT[5];
		else if(xdiff==-1 && ydiff == 1)
			sensorAT = obs->AT[6];
		else if(xdiff==-1 && ydiff == 0)
			sensorAT = obs->AT[7];
		else if(xdiff==-1 && ydiff == -1)
			sensorAT = obs->AT[8];

		ObservedRLocation = true;
		if(sensorAT == 1){ //Reward location directly observed
			LeftR = false;	
		}
		else{
			LeftR = true;	
		}
	}


}

