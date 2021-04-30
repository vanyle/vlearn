#include "NeuralNetwork.h"

#include "math/math.h"
#include "utils/utils.h"

namespace vio{


	float L2errorFn(const Vector& in,const Vector& expected){
		Vector r = Vector::sub(in,expected);
		return r.norm();
	}
	Vector L2errorDerivativeFn(const Vector& in,const Vector& expected){
		Vector r = Vector::sub(in,expected);
		if(r.normSquared() < 0.00001){
			r.fill(0);
			return r;
		}
		r /= r.norm();
		return r;
	}

	float crossEntropyErrorFn(const Vector& in,const Vector& expected){
		// in=p, expected = q
		// apply softmax to in.
		float r = 0;
		for(u32 i = 0;i < in.size();i++){
			r -= expected.get(i) * log2(in.get(i));
		}
		return r;
	}
	Vector crossEntropyErrorDerivative(const Vector& in,const Vector& expected){
		return Vector::sub(in,expected);
	}


	NeuralNetwork::NeuralNetwork(){
		this->errorFunction = L2errorFn;
		this->errorFunctionGradient = L2errorDerivativeFn;
		computationFunction = 0;
	}
	NeuralNetwork::~NeuralNetwork(){}

	Vector NeuralNetwork::apply(const Vector& v){
		Vector res(v);
		u32 i = 0;
		for(Layer* l : layers){
			i++;
			res = std::move(l->apply(res));
		}
		return res;
	}


	float NeuralNetwork::loss(std::vector<Vector>& in,std::vector<Vector>& out){
		vassert(in.size() == out.size());
		float t = 0;
		for(u32 i = 0;i < in.size();i++){
			t += this->errorFunction(apply(in[i]),out[i]);
		}
		return t / in.size(); // avg error
	}

	void NeuralNetwork::prepare(){
		// TODO allocate memory for the training
		isReady = true;
		for(u32 i = 0;i < layers.size()-1;i++){
			if(layers[i]->outputSize() != layers[i+1]->inputSize()){
				vpanic("Layers are misshaped, layer %i has outputSize %i but layer %i has inputSize %i !",
					i,layers[i]->outputSize(),i+1,layers[i+1]->inputSize());
			}
		}
	}

	// a classic implementation of the computation function for a CPU backend.
	// this function is (almost) pure and can be run on many thread by changing the start and end indices.
	Matrix cpuComputationFunction(NeuralNetwork& ref,u32 tstart,u32 tend,std::vector<Vector>& in,std::vector<Vector>& out,const std::vector<u32> permutationTable){
		Matrix m(1,1);
		m.fill(0);

		return m;
	}

	void NeuralNetwork::train(std::vector<Vector>& in,std::vector<Vector>& out,float learningRate){
		if(!isReady){
			vpanic("The neural network is not ready! Call neuralnetwork.prepare() first!");
		}
		vassert(in.size() == out.size());

		// TODO Split this into multiple files.

		// TODO The amount of memory allocated by train for all the vectors and stuff is crazy.
		// We need to use a custom allocator that would call free and new only once during the whole training session.
		// Indeed the amount of memory needed to run this function is deterministic and depends only on the dimensions of the network,
		// not on the values it contains.

		// TODO save a model

		// TODO Abstract Parallelism : an interface to represent ML tasks

		// shuffle in and out using a permutation.
		std::vector<u32> permutation;
		for(u32 i = 0;i < in.size();i++){
			permutation.push_back(i);
		}
		for(u32 i = 0;i < in.size();i++){
			// do the swaps.
			u32 temp = permutation[i];
			u32 swap_index = randomU32(in.size()-1);
			permutation[i] = permutation[swap_index];
			permutation[swap_index] = temp;
		}

		u32 batchSize = in.size() / computationCoreCount;

		for(u32 train_index = 0;train_index < in.size();train_index++){
			u32 real_index = permutation[train_index];
			// Evaluate the intermediate results for every layer.
			// intermediate.size == layer.size + 1
			std::vector<Vector> intermediate{ in[real_index] };
			for(u32 j = 0;j < layers.size();j++){
				intermediate.push_back(layers[j]->apply(intermediate[intermediate.size() - 1]));
			}

			for(u32 i = 0;i < layers.size();i++){
				if(!layers[i]->isLearnable()) continue;

				// Gradient computation starts here:

				// J/dx * dx/dm = J/dm (the thing we wanna compute). We know that dx/dm = transpose(v)
				
				// Let's compute v2 = J/dx (it's a vector)
				Vector v2 = this->errorFunctionGradient(intermediate[intermediate.size() - 1 ],out[real_index]);
				if(v2.normSquared() == 0.00){
					continue;
				}

				for(i32 j = layers.size()-1;j > (i32)i;j--){ // j in [ 1;layer.size() ]
					v2 = std::move(layers[j]->applyGradient(v2,intermediate[j],intermediate[j-1]));
				}

				// Matrix totalGradient(layers[i]->inputSize(),layers[i]->outputSize());
				Matrix totalGradient = Vector::crossNorm(v2,intermediate[i]); // J/dx * dx/dm = J/dm

				totalGradient *= learningRate;

				// adam implementation
				// adam requests us to store the gradients from the previous steps inside moment vectors.
				// this would triple the RAM usage.
				// how to deal with this ?

				layers[i]->updateMatrix(totalGradient); // adjuste the layer based on the average gradient.

				if(layers[i]->isBias()){
					v2 *= learningRate;
					layers[i]->updateBias(v2);
				}
			}
		}

	}

}
