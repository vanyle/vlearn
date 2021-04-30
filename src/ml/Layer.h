/*
 * Layer.h
 *
 *  Created on: 30 avr. 2021
 *      Author: vanyle
 */

#pragma once

#include "math/vector.h"
#include "utils/utils.h"

namespace vio {

/**
Layer is an abstract class (interface) representing a Layer of a neural network.

Implementations of Layer include:
	- DenseLayer for "classic" layers performing a linear transformation and applying an activation function
	- ConvLayer for convolutions
	- BatchNormLayer for normalizations
	- SoftMaxLayer for applying softmax
	- etc ...

You can implement your own layer, you just need to implement the following methods:
@code

YourLayer(); // constructor that sets inputSize, outputSize, learnable and bias
void print(); // used for debug, display your layer to stdout
Vector apply(const Vector& in); // evaluates the layer
Vector applyGradient(const Vector& in,const Vector& evaluationPosition,const Vector& previousEvaluationPosition); // gradient computation

void updateMatrix(const Matrix& m); // update the weights, not needed if learable = false, in this case, vassert(false) in this.
void updateBias(const Vector& v); // update the bias, not needed if bias = false, in this case, vassert(false) in this.

@endcode
 */

class Layer{
	protected:
		u32 inS;
		u32 outS;

		// set to false to set the weights
		// useful for finetuning.
		bool learnable = false;
		bool bias = false;
	public:
		Layer() = delete;
		Layer(u32 inputSize,u32 outputSize);
		virtual ~Layer();

		bool isLearnable();
		bool isBias();

		u32 inputSize();
		u32 outputSize();

		virtual void print(); // for debug

		// virtual std::string serialize();

		virtual Vector apply(const Vector& in) = 0;

		// same as apply but in the other direction.
		// used for backprop, usually implemented with m.applyTranspose
		// This is the gradient of the network at a given position.
		virtual Vector applyGradient(const Vector& in,const Vector& evaluationPosition,const Vector& previousEvaluationPosition) = 0;

		// usually does this->m -= m.
		// not always thou (for example, in conv layers, this is not the case.)
		virtual void updateMatrix(const Matrix& m);
		virtual void updateBias(const Vector& v);
	};

}
