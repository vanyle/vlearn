/*
 * Optimizer.h
 *
 *  Created on: 30 avr. 2021
 *      Author: vanyle
 */

#pragma once

#include <vector>
#include "math/Vector.h"
#include "math/Matrix.h"
#include "Layer.h"

namespace vio {

class Optimizer{
public:
	Optimizer();
	virtual ~Optimizer();

	virtual void prepare(std::vector<Layer*>& layers);
	virtual void adjustGradientMatrix(Matrix& gradm);
};

class ConstantOptimizer : Optimizer{
	float learningRate = 0.001;
public:
	ConstantOptimizer();
	~ConstantOptimizer();

	void prepare(std::vector<Layer*>& layers);
	void adjustGradientMatrix(Matrix& gradm);
};
class AdamOptimizer : Optimizer{
	float learningRate = 0.001;
	// adam parameters taken from: https://arxiv.org/pdf/1412.6980.pdf
	float beta1 = 0.9;
	float beta2 = 0.999;
	// Same size as the number of learnable layers (obtained with prepare)
	std::vector<Matrix> firstOrderMoment; // m_t = beta1 * m_t-1 + (1 - beta1) * gradientMatrix
	std::vector<Matrix> secondOrderMoment; // v_t = beta2 * v_t-1 + (1 - beta2) * gradientMatrix^2 (element wise multiplication)
	// return learningRate * m_t / (1-beta1^t) / (sqrt(v_t / (1-beta2^t)) + 0.000001)
public:
	AdamOptimizer(float beta1 = 0.9,float beta2 = 0.999);
	~AdamOptimizer();

	void prepare(std::vector<Layer*>& layers);
	void adjustGradientMatrix(Matrix& gradm);
};

} /* namespace vio */

