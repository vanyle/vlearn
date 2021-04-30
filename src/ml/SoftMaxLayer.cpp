/*
 * SoftMaxLayer.cpp
 *
 *  Created on: 30 avr. 2021
 *      Author: vanyle
 */

#include "SoftMaxLayer.h"

namespace vio {

	// Softmax layer implementation
	// Used as the last layer to convert everything to a probability distribution
	// Currently, the backpropagation part (applyGradient) is incorrect. This is a bug, I suck at math :(

	SoftMaxLayer::SoftMaxLayer(u32 inputSize) : Layer(inputSize,inputSize){
		this->bias = false;
		this->learnable = false;
	}
	SoftMaxLayer::~SoftMaxLayer(){

	}
	void SoftMaxLayer::print(){
		debug("Softmax Layer: %i",inS);
	}
	Vector SoftMaxLayer::apply(const Vector& x){
		return x.softmax();
	}
	Vector SoftMaxLayer::applyGradient(const Vector& in,const Vector& evaluationPosition,const Vector& previousEvaluationPosition){
		// dy_i/dz_j = -y_i * y_j.
		Matrix m(in.size(),in.size());
		for(u32 i = 0;i < in.size();i++){
			for(u32 j = 0;j < in.size();j++){
				if(i!=j){
					m.at(i,j) = -evaluationPosition.get(i) * evaluationPosition.get(j);
				}else{
					m.at(i,i) = (1-evaluationPosition.get(i))*evaluationPosition.get(i);
				}
			}
		} // compute jacobian of layer, than multiply it.
		return m.applyTranspose(in);
	}
	void SoftMaxLayer::updateMatrix(const Matrix& m){vassert(false);}
	void SoftMaxLayer::updateBias(const Vector& v){vassert(false);}

} /* namespace vio */
