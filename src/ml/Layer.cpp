/*
 * Layera.cpp
 *
 *  Created on: 30 avr. 2021
 *      Author: vanyle
 */

#include "Layer.h"

namespace vio {

// Layer implementation
Layer::Layer(u32 inputSize,u32 outputSize){
	this->inS = inputSize;
	this->outS = outputSize;
}
Layer::~Layer(){}

bool Layer::isLearnable(){
	return learnable;
}
bool Layer::isBias(){
	return bias;
}
void Layer::updateMatrix(const Matrix& m){}
void Layer::updateBias(const Vector& v){}
u32 Layer::inputSize(){
	return inS;
}
u32 Layer::outputSize(){
	return outS;
}
void Layer::print(){
	debug("Layer %i x %i (unknown type)",this->inS,this->outS);
}

} /* namespace vio */
