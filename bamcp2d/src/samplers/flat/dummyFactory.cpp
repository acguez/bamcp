#include "dummyFactory.h"


DummyFactory::DummyFactory(SIMULATOR2D* _real): real(_real){

}

SamplerFactory2D* DummyFactory::createNewClone(){
	return new DummyFactory(real);
}

void DummyFactory::Clone(SamplerFactory2D* parent){
	real = ((DummyFactory*)parent)->real;
}

void DummyFactory::render(double x,double y,double sqsz){}

Sampler2D* DummyFactory::getMDPSampler(double gamma){
	return new DummySampler(real);
}
