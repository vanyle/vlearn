/*
 * DenseLayer.h
 *
 *  Created on: 30 avr. 2021
 *      Author: vanyle
 */

#pragma once

#include "Layer.h"
#include "math/Matrix.h"
#include "math/Vector.h"
#include "utils/utils.h"

namespace vio {

/**
	 This is similar to a Dense Keras layer.
	 It supports multiple activation functions but uses leaky relu by default.
	 Example:
	 @code
	NeuralNetwork nn;
	DenseLayer l1(3,5,"leakyrelu"); // inputsize, outputsize, activation function
	DenseLayer l2(5,2,"leakyrelu"); // inputsize, outputsize, activation function
	nn.layers.push_back(l1);
	nn.layers.push_back(l2);
	nn.prepare();
	// You are ready to train / use the network now !
	// It takes 3 numbers as input and outputs 2 numbers (a vector of size 2)
	 @endcode
	*/
	class DenseLayer : public Layer{
	private:
		Matrix m;
		Vector b;
		float (*activatorFn)(float);
		float (*activatorDerivativeFn)(float);

	public:
		DenseLayer(u32 inputSize,u32 outputSize,const std::string& activator = "leakyrelu");
		~DenseLayer();

		void print();

		// function every layer has to implement
		Vector apply(const Vector& in);

		// used for gradient backpropagation
		Vector applyGradient(const Vector& in,const Vector& evaluationPosition,const Vector& previousEvaluationPosition);

		// in case the layer is non learnable, those 2 functions are never called, they
		// are used to update the weights
		void updateMatrix(const Matrix& m);
		void updateBias(const Vector& vec);

		void randomInit(float dev,float mean = 0);
	};
} /* namespace vio */

