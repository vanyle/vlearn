/*
 * DenseLayer.cpp
 *
 *  Created on: 30 avr. 2021
 *      Author: vanyle
 */

#include "DenseLayer.h"
#include "math/math.h"

namespace vio {

// The activator functions
float reluFn(float f){
	return f > 0 ? f : 0;
}
float reluDerivativeFn(float f){
	return f > 0 ? 1 : 0;
}
float leakyReluFn(float f){
	return f > 0 ? f : 0.01*f;
}
float leakyReluDerivativeFn(float f){
	return f > 0 ? 1 : 0.01;
}
float sigmoidFn(float f){
	return 1 / (1 + std::exp(-f));
}
float sigmoidDerivativeFn(float f){
	return f*(1-f); // this is sigmoid'(x) = sigmoid(x) * (1 - sigmoid(x))
}
float tanhFn(float f){
	return std::tanh(f);
}
float tanhDerivativeFn(float f){
	return 1-f*f; // tanh' = 1 - tanh(f)**2
}
float softmaxFn(float f){
	return std::log(1+std::exp(f));
}
float softmaxDerivativeFn(float f){
	return std::exp(-f);
}

typedef float (*mathFn)(float);
mathFn getActivator(const std::string& actName){
	if(actName == "sigmoid"){
		return sigmoidFn;
	}else if(actName == "tanh"){
		return tanhFn;
	}else if(actName == "relu"){
		return reluFn;
	}else if(actName == "softmax"){
		return softmaxFn;
	}else{ // leaky relu.
		return leakyReluFn;
	}
}
mathFn getActivatorDerivative(const std::string& actName){
	if(actName == "sigmoid"){
		return sigmoidDerivativeFn;
	}else if(actName == "tanh"){
		return tanhDerivativeFn;
	}else if(actName == "relu"){
		return reluDerivativeFn;
	}else if(actName == "softmax"){
		return softmaxDerivativeFn;
	}else{ // leaky relu.
		return leakyReluDerivativeFn;
	}
}

DenseLayer::DenseLayer(u32 inputSize,u32 outputSize,const std::string& activator) : Layer(inputSize,outputSize),m(inputSize,outputSize),b(outputSize){
	activatorFn = getActivator(activator);
	activatorDerivativeFn = getActivatorDerivative(activator);
	this->learnable = true;
	this->bias = true;
	this->b.fill(0); // it is common to init the biases at 0 at the beginning.
}
DenseLayer::~DenseLayer(){

}
void DenseLayer::print(){
	debug("Layer %i x %i (dense)",this->inS,this->outS);
	debug("m=");
	m.print();
	debug("b=");
	b.print();
}
Vector DenseLayer::apply(const Vector& x){
	vassert(x.size() == inS);
	// y = relu(mx)
	Vector y = m.apply(x);
	y += b;
	for(u32 i = 0;i < y.size();i++){
		y.at(i) = activatorFn(y.get(i));
	}
	return y;
}
Vector DenseLayer::applyGradient(const Vector& in,const Vector& evaluationPosition,const Vector& unused){
	Vector r = m.applyTranspose(in);
	// Note that the evaluation position is the once after the layer has been applied.
	for(u32 x = 0;x < r.size();x++){
		r.at(x) *= this->activatorDerivativeFn(evaluationPosition.get(x));
	}
	return r;
}
void DenseLayer::updateBias(const Vector& vec){
	this->b -= vec;
}
void DenseLayer::updateMatrix(const Matrix& um){
	this->m -= um;
}
void DenseLayer::randomInit(float dev,float mean){
	this->m.fillRandom(dev,mean); // r = (x-.5) * 2 * dev + mean
}

} /* namespace vio */
