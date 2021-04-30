/*
 * SoftMaxLayer.h
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

// non learnable layer, used as a last layer to turn the input into a probability distribution.
// res <- in.softmax(); (exp / sum of exp)
// gradient: matrix where a_ij = in_i * res_j
class SoftMaxLayer : public Layer{
public:
	SoftMaxLayer(u32 inputSize); // in = out
	~SoftMaxLayer();

	Vector apply(const Vector& in);
	Vector applyGradient(const Vector& in,const Vector& evaluationPosition,const Vector& previousEvaluationPosition); // let s = softmax(intermediate), and A :=  -s_i * s_j, then return A*s;
	void print();

	void updateMatrix(const Matrix& m); // does nothing
	void updateBias(const Vector& v); // does nothing
};

} /* namespace vio */
