/*
 * BatchNormLayer.h
 *
 *  Created on: 30 avr. 2021
 *      Author: vanyle
 */

#pragma once

#include "Layer.h"
#include "math/Vector.h"
#include "math/Matrix.h"
#include "utils/utils.h"

namespace vio {

// non learnable layer, normalizes its input for the next layer.
// res = (res - E(res)) / sqrt(var(res))
class BatchNormLayer : public Layer{
public:
	BatchNormLayer(u32 inputSize); // in = out
	~BatchNormLayer();

	Vector apply(const Vector& in);
	Vector applyGradient(const Vector& in,const Vector& evaluationPosition,const Vector& previousEvaluationPosition); // let s = softmax(intermediate), and A :=  -s_i * s_j, then return A*s;
	void print();

	void updateMatrix(const Matrix& m); // does nothing
	void updateBias(const Vector& v); // does nothing
};

} /* namespace vio */

