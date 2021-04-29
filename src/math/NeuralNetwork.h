// Technicaly math.
#pragma once

#include <any>
#include "Vector.h"
#include "Matrix.h"
#include <vector>
#include <string>

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

		//virtual void randomInit(float mean = 0,float dev = 1); // uniform random initiation with mean= avg, and std deviation = dev
		virtual void print(); // for debug

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

	/**
		 This is similar to a Conv2D Keras layer

		 a conv layer takes a X input and applies only one small x matrix to it.

		outputSize = floor(inputSize / reductionFactor / reductionFactor)
		I recommend to choose inputSize and reductionFactor so that inputSize / reductionFactor / reductionFactor is an integer.

		The input is assumed to be a SQUARE image. Its square root has to be an integer.
		The ouput is also assumed to be a square.

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

		void setKernel(Matrix k); // mostly needed for debug.
		Matrix& getKernel(); // mostly needed for the cool dreamy animations

		void randomInit(float dev,float mean = 0.f);

		Vector apply(const Vector& in);
		Vector applyGradient(const Vector& in,const Vector& evaluationPosition,const Vector& previousEvaluationPosition);

		void updateMatrix(const Matrix& m);
		void updateBias(const Vector& v); // does nothing
	};

	/**
	Represents a NeuralNetwork.
	See DenseLayer for an example of how to use it.
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
		NeuralNetwork();
		~NeuralNetwork();

		u32 computationCoreCount = 4; // number of operation the machine is able to do at the same time.

		// an implementation of the computation, depending on the computing device used.
		Matrix (*computationFunction)(NeuralNetwork& ref,u32 tstart,u32 tend,std::vector<Vector>& in,std::vector<Vector>& out,const std::vector<u32> permutationTable);

		std::vector<Layer*> layers;

		void train(std::vector<Vector>& in,std::vector<Vector>& out,float rate = 0.01);

		// function applied to the last layer for gradient descent training.
		// example (L2): norm(input - output)
		// gradient of example: (input - output) / norm(input - output)
		// If you cannot compute the gradient / error for the pair given (if the function used is not continuous at that point or something),
		// return 0 and it will be ignored.
		float (*errorFunction)(const Vector& input,const Vector& expected) = 0;
		Vector (*errorFunctionGradient)(const Vector& input,const Vector& expected) = 0;

		float loss(std::vector<Vector>& in,std::vector<Vector>& out);

		void prepare(); // all this when ready, this will allocate the memory required by the network for fast trainign.
		void ready(); // free the memory taken by prepare.

		Vector apply(const Vector& in);

		std::string serialize(); // TODO, used to save/load a trained network.
		void load(std::string s);
	};

}
