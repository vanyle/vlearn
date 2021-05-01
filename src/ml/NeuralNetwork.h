// Technicaly math.
#pragma once

#include <any>
#include "math/Vector.h"
#include "math/Matrix.h"
#include <vector>
#include <string>
#include "Layer.h"

namespace vio{

	float crossEntropyErrorFn(const Vector& in,const Vector& expected);
	Vector crossEntropyErrorDerivative(const Vector& in,const Vector& expected);

	// Used to update a learnable layer with bias.
	struct UpdatePair{
		Matrix m;
		Vector v;
	};

	/**
	Represents a NeuralNetwork.
	See DenseLayer for an example of how to use it.

	Here is a description of the theoretical desscription of how this work.

	A Network is list of Layers. A Layer describes an operation on a Vector.
	By stacking various layers, you compose the operation together to create the network,
	which you can see as a complicated function.

	The layers also have function that "returns their derivative".
	Because their derivative is a Matrix, (the jacobian of a function from Vectors to Vectors is a matrix),
	the layers provide a function that multiplies a vector of your choice with their jacobian without
	ever providing you their jacobian.

	This is because their jacobian can be quite big and those matrices are often filled with zeroes and take a lot of space in memory.
	By doing this, the layers can, internally, never store the jacobian directly.

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
