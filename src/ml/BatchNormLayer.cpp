/*
 * BatchNormLayer.cpp
 *
 *  Created on: 30 avr. 2021
 *      Author: vanyle
 */

#include "BatchNormLayer.h"
#include <cmath>

namespace vio {

	// "Batchnorm" layer implementation
	// Used to normalize input so that it matches a normal disturbution (according to central limit theorem)

	BatchNormLayer::BatchNormLayer(u32 inputSize) : Layer(inputSize,inputSize){
		this->bias = false;
		this->learnable = false;
	}
	BatchNormLayer::~BatchNormLayer(){}
	Vector BatchNormLayer::apply(const Vector& x){
		float e = 0;
		for(u32 i = 0;i < x.size();i++){
			e += x.get(i);
		}
		e /= x.size();
		float v = 0;
		for(u32 i = 0;i < x.size();i++){
			float t = e - x.get(i);
			v += t*t;
		}
		v = std::sqrt(v);
		Vector r(x.size());
		for(u32 i = 0;i < x.size();i++){
			r.at(i) = (x.get(i) - e) / v; // standard score
		}

		return r;
	}
	Vector BatchNormLayer::applyGradient(const Vector& in,const Vector& evaluationPosition,const Vector& pep){
		float e = 0;
		for(u32 i = 0;i < pep.size();i++){
			e += pep.get(i);
		}
		e /= pep.size();
		float v = 0;
		for(u32 i = 0;i < pep.size();i++){
			float t = e - pep.get(i);
			v += t*t;
		}
		v = std::sqrt(v);
		Vector r(in);
		r /= v;
		return r;
	}
	void BatchNormLayer::print(){
		debug("BatchNorm Layer: %i",inS);
	}
	void BatchNormLayer::updateMatrix(const Matrix& m){vassert(false);}
	void BatchNormLayer::updateBias(const Vector& v){vassert(false);}

} /* namespace vio */
