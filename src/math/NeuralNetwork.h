// Technicaly math.
#pragma once

#include <any>
#include "Vector.h"
#include "Matrix.h"
#include <vector>

namespace vio{


	/**
	Layer is an abstract class representing a Layer of a neural network.
	Implementations of Layer include:
		- SoftReluLayer for "classic" layers performing a linear transformation and applying an activation function
		- ConvLayer for convolutions
		- BatchNormLayer for normalizations
		- ActiFunction for activator functions
		- ...
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
		void randomInit(float mean = 0,float dev = 1); // uniform random initiation with mean= avg, and std deviation = dev
		virtual void print(); // for debug

		virtual Vector apply(const Vector& in) = 0;

		// same as apply but without the activator function and in the other direction.
		// used for backprop, usually implemented with m.applyTranspose
		virtual Vector linApplyTranspose(const Vector& in) = 0;

		virtual float activatorDerivative(float f) = 0;

		// usually does this->m -= m.
		// not always thou (for example, in conv layers, this is not the case.)
		virtual void updateMatrix(const Matrix& m) = 0;
		virtual void updateBias(const Vector& v) = 0;
	};

	// non learnable layer, normalizes its input for the next layer.
	// res = (res - E(res)) / sqrt(var(res))
	class BatchNorm : public Layer{

	};

	/**
	 This is similar to a Dense Keras layer with a leaky relu.
	 Note: make this more general with multiple activation functions using functions pointers.
	 Needs support: sigmoid, tanh, relu, leakyrelu
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

		// in case the layer is non learnable, those 4 matter much less,
		// they are used for back-prop.
		Vector linApplyTranspose(const Vector& in);
		float activatorDerivative(float f);

		void updateMatrix(const Matrix& m);
		void updateBias(const Vector& vec);

		void randomInit(float dev,float mean = 0);
	};

	/**
		 This is similar to a Conv2D Keras layer

		 a conv layer takes a X input and applies only one small x matrix to it.
		 sqrt(X) and sqrt(x) must be integers.
		 X is reshaped internally to be thought of as a square
		 sqrt(x) must divide sqrt(X) evenly.
		 outputSize = inputSize / reductionFactor / reductionFactor
		 I recommend inputSize to be a power of 2 or at least by divisible by 16 (=4*4)
		 Remember that outputSize needs to be an integer.
	 */
	class ConvLayer : public Layer{
	private:
		Matrix kernel;
		u32 side_length;
		u32 reduc;
	public:
		ConvLayer(u32 inputSize,u32 reductionFactor,u32 kernel_size_x = 8,u32 kernel_size_y = 8);
		~ConvLayer();

		void print();

		Vector apply(const Vector& in);
		Vector linApplyTranspose(const Vector& in);
		float activatorDerivative(float f);
		void updateMatrix(const Matrix& m);
		// no biases for this layer type.
	};

	/**
	Represents a NeuralNetwork.

	 */
	class NeuralNetwork{
	private:
		// Stuff to put inside memory:
		// totalGradient = max(inputSize,outputSize) over layers
		// intermediate = sum(inputSize) over layers + outputSize of last layer
		// v2 = max(outputSize) over layers
		// v3 = max(outputSize) over layers (temp space where v2 is copied for intermediate steps)
		void *memory = 0;
		bool isReady = false;
	public:
		std::vector<Layer*> layers;

		void train(std::vector<Vector> in,std::vector<Vector> out,float rate = 0.01);
		float RMSerror(std::vector<Vector> in,std::vector<Vector> out);

		void prepare(); // all this when ready, this will allocate the memory required by the network for fast trainign.
		void ready(); // free the memory taken by prepare.

		Vector apply(const Vector& in);
	};

}
