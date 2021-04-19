#include "NeuralNetwork.h"
#include "math.h"
#include "utils/utils.h"

namespace vio{

	// The activator functions
	float reluFn(float f){
		return f > 0 ? f : 0;
	}
	float reluDerivativeFn(float f){
		return f > 0 ? 1 : 0;
	}
	float leakyReluFn(float f){
		return f > 0 ? f : 0.01*f;
	}
	float leakyReluDerivativeFn(float f){
		return f > 0 ? 1 : 0.01;
	}
	float sigmoidFn(float f){
		return 1 / (1 + std::exp(-f));
	}
	float sigmoidDerivativeFn(float f){
		return f*(1-f); // this is sigmoid'(x) = sigmoid(x) * (1 - sigmoid(x))
	}
	float tanhFn(float f){
		return std::tanh(f);
	}
	float tanhDerivativeFn(float f){
		return 1-f*f; // tanh' = 1 - tanh(f)**2
	}
	float softmaxFn(float f){
		return std::log(1+std::exp(f));
	}
	float softmaxDerivativeFn(float f){
		return std::exp(-f);
	}

	typedef float (*mathFn)(float);
	mathFn getActivator(const std::string& actName){
		if(actName == "sigmoid"){
			return sigmoidFn;
		}else if(actName == "tanh"){
			return tanhFn;
		}else if(actName == "relu"){
			return reluFn;
		}else if(actName == "softmax"){
			return softmaxFn;
		}else{ // leaky relu.
			return leakyReluFn;
		}
	}
	mathFn getActivatorDerivative(const std::string& actName){
		if(actName == "sigmoid"){
			return sigmoidDerivativeFn;
		}else if(actName == "tanh"){
			return tanhDerivativeFn;
		}else if(actName == "relu"){
			return reluDerivativeFn;
		}else if(actName == "softmax"){
			return softmaxDerivativeFn;
		}else{ // leaky relu.
			return leakyReluDerivativeFn;
		}
	}

	// Layer implementation
	Layer::Layer(u32 inputSize,u32 outputSize){
		this->inS = inputSize;
		this->outS = outputSize;
	}
	Layer::~Layer(){}
	bool Layer::isLearnable(){
		return learnable;
	}
	bool Layer::isBias(){
		return bias;
	}
	u32 Layer::inputSize(){
		return inS;
	}
	u32 Layer::outputSize(){
		return outS;
	}
	void Layer::print(){
		debug("Layer %i x %i (unknown type)",this->inS,this->outS);
	}

	// ---------------------
	DenseLayer::DenseLayer(u32 inputSize,u32 outputSize,const std::string& activator) : Layer(inputSize,outputSize),m(inputSize,outputSize),b(outputSize){
		activatorFn = getActivator(activator);
		activatorDerivativeFn = getActivatorDerivative(activator);
		this->learnable = true;
		this->bias = true;
		this->b.fill(0); // it is common to init the biases at 0 at the beginning.
	}
	DenseLayer::~DenseLayer(){

	}
	void DenseLayer::print(){
		debug("Layer %i x %i (dense)",this->inS,this->outS);
		debug("m=");
		m.print();
		debug("b=");
		b.print();
	}
	float DenseLayer::activatorDerivative(float reluv){ // softrelu'(x) as a function of softrelu(x)
		return activatorDerivativeFn(reluv);
	}
	Vector DenseLayer::apply(const Vector& x){
		// y = relu(mx)
		Vector y = m.apply(x);
		y += b;
		for(u32 i = 0;i < y.size();i++){
			y.at(i) = activatorFn(y.get(i));
		}
		return y;
	}
	Vector DenseLayer::linApplyTranspose(const Vector& in){
		return m.applyTranspose(in);
	}
	void DenseLayer::updateBias(const Vector& vec){
		this->b -= vec;
	}
	void DenseLayer::updateMatrix(const Matrix& um){
		this->m -= um;
	}
	void DenseLayer::randomInit(float dev,float mean){
		this->m.fillRandom(dev,mean); // r = (x-.5) * 2 * dev + mean
	}

	// Convolution layer implementation
	// This only works on gray-scaled images.
	// See SeparableConv2d for color.
	ConvLayer::ConvLayer(u32 inputSize,u32 reductionFactor,u32 kernel_size_x,u32 kernel_size_y) :
			Layer(inputSize,inputSize / reductionFactor / reductionFactor),
			kernel(kernel_size_x,kernel_size_y) {
		this->side_length = std::sqrt(inputSize);
		this->reduc = reductionFactor;
		this->learnable = true;
		this->bias = false;
		vassert(side_length*side_length == inputSize); // check input is a square
	}
	ConvLayer::~ConvLayer(){

	}
	void ConvLayer::print(){
		debug("Conv2D Layer %i x %i with kernel %i x %i",this->inputSize(),this->outputSize(),kernel.width(),kernel.height());
		kernel.print();
	}
	Vector ConvLayer::apply(const Vector& x){
		Vector y(inS / reduc / reduc);
		u32 si = 0; // small i, si = i / reduc
		u32 sj = 0;

		for(u32 i = 0;i < side_length;i += reduc,si ++){
			for(u32 j = 0;j < side_length;j += reduc,sj ++){
				// apply conv matrix around x.at(i*size_length+j);
				float ytemp = 0;
				for(u32 ki = 0;ki < kernel.width();ki++){
					for(u32 kj = 0;kj < kernel.height();kj++){
						if(i+ki >= side_length || j+kj >= side_length) continue;
						ytemp += x.get((i+ki)*side_length+(j+kj)) * kernel.get(kj,ki);
					}
				}
				y.at(si*(side_length/reduc) + sj) = ytemp;
			}
		}

		for(u32 i = 0;i < y.size();i++){
			y.at(i) /= (y.at(i) < 0) ? 100 : 1;
		}
		return x;
	}
	Vector ConvLayer::linApplyTranspose(const Vector& in){
		vassert(in.size() == outputSize()); // reverse direction from apply.
		Vector res(inputSize());
		res.fill(0); // outputSize = inputSize / reduc / reduc

		const i32 ssl = side_length / reduc; // small side length,

		for(u32 i = 0;i < side_length;i++){
			for(u32 j = 0;j < side_length;j++){
				for(i32 si = max(0,(i32)((i-kernel.height()) / reduc));si < min(ssl,floor(i/reduc)+1);si++){
					for(i32 sj = max(0,(i32)((j-kernel.height()) / reduc));sj < min(ssl,floor(j/reduc)+1);sj++){
						const i32 kernelPosJ = j-sj*reduc;
						const i32 kernelPosI = i-si*reduc;
						// assert should not trigger unless i fucked up the loop indices.
						vassert((kernelPosI>0 && kernelPosI < (i32)kernel.width() && kernelPosJ > 0 && kernelPosJ < (i32)kernel.height()));
						res.at(i*side_length+j) += kernel.get(kernelPosJ,kernelPosI) * in.get(si*ssl + sj);
					}
				}
			}
		}

		return res;
	}
	float ConvLayer::activatorDerivative(float reluv){
		return (reluv < 0) ? 0.01 : 1;
	}
	void ConvLayer::updateMatrix(const Matrix& m){
		// m is of size inputSize * outputSize, this is bigger than the kernel.
		// we need to average the coefs of m in a repeated pattern to update the kernel
		// also note that most of the elements of m are 0.
		// this is inefficient (a sparse matrix would be better)
		// however, this would reduce genericity.
		Matrix kernelUpdate(kernel.width(),kernel.height());
		kernelUpdate.fill(0);
		const u32 ssl = side_length / reduc;

		for(u32 i = 0;i < side_length;i ++){
			for(u32 j = 0;j < side_length;j++){
				for(i32 si = max(0,(i32)((i-kernel.height()) / reduc));si < min(ssl,floor(i/reduc)+1);si++){
					for(i32 sj = max(0,(i32)((j-kernel.height()) / reduc));sj < min(ssl,floor(j/reduc)+1);sj++){
						const i32 kernelPosJ = j-sj*reduc;
						const i32 kernelPosI = i-si*reduc;
						kernelUpdate.at(kernelPosJ,kernelPosI) += m.get(si*ssl+j,i*side_length+j);
					}
				}
			}
		}

		kernelUpdate *= reduc/side_length;
		kernel -= kernelUpdate;
	}

	// -----------------------------

	Vector NeuralNetwork::apply(const Vector& v){
		Vector res(v);
		u32 i = 0;
		for(Layer* l : layers){
			i++;
			res = std::move(l->apply(res));
		}
		return res;
	}

	// Root mean square
	float NeuralNetwork::RMSerror(std::vector<Vector> in,std::vector<Vector> out){
		vassert(in.size() == out.size());
		float t = 0;
		for(u32 i = 0;i < in.size();i++){
			Vector r = apply(in[i]);
			r -= out[i];
			t += r.normSquared();
		}
		return std::sqrt(t / in.size());
	}

	void NeuralNetwork::prepare(){
		// TODO check that the layers make sense: layer[i].outputSize == layer[i+1].inputSize
		isReady = true;
		for(u32 i = 0;i < layers.size()-1;i++){
			if(layers[i]->outputSize() != layers[i+1]->inputSize()){
				vpanic("Layers are misshaped, layer %i has outputSize %i but layer %i has inputSize %i !",
					i,layers[i]->outputSize(),i+1,layers[i+1]->inputSize());
			}
		}
	}

	void NeuralNetwork::train(std::vector<Vector> in,std::vector<Vector> out,float learningRate){
		if(!isReady){
			vpanic("The neural network is not ready! Call neuralnetwork.prepare() first!");
		}
		vassert(in.size() == out.size());

		// gradient computation reference:
		// https://web.stanford.edu/class/cs224n/readings/gradient-notes.pdf

		// Issues and stuff to do:

		// 3. The amount of memory allocated by train for all the vectors and stuff is crazy.
		// We need to use a custom allocator that would call free and new only once during the whole training session.
		// Indeed the amount of memory needed to run this function is deterministic and depends only on the dimensions of the network,
		// not on the values it contains.

		// 4. Parallelism.

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

				// Because RAM is not infinite (I don't think we can hold the whole model in ram at once)
				// we operate on one layer at a time.
				// we consider the model to be a function E, so that:
				// E_in_out_N (layer) = || f(in) - out || = (f(in) - out) / || f(in) - out  || * grad f(in)
				// i.e the only parameter is the current layer.
				// we need to compute the gradient of E.
				Matrix totalGradient(layers[i]->inputSize(),layers[i]->outputSize());
				totalGradient.fill(0);

				// compute the gradient of the rest of the network computed at point v.
				// This is a tensor, to avoid computing it, we compute the gradient of the other layers and then compute:
				// J/dx * dx/dm = J/dm (the thing we wanna compute). We know that dx/dm = transpose(v)
				
				// Let's compute v2 = J/dx (it's a vector)
				Vector v2 = Vector::sub(intermediate[intermediate.size() - 1 ],out[real_index]);
				if(v2.normSquared() < 0.00001){
					continue;
				}
				v2 /= v2.norm(); // change this part based on the error function at the end. This is L2 norm

				for(i32 j = layers.size()-1;j > (i32)i;j--){
					// debug("(i=%i,j=%i)Layer h = %i ; v.size = %i",i,j,layers[j].m.height(),v2.size());
					v2 = std::move(layers[j]->linApplyTranspose(v2));
					// we don't have intermediate value before the activator function is apply
					// but after the multiplication by the matrix of the layer.
					// This is not an issue as relu'(x) = sign(relu(x)) (relu(x) is known, not x)
					for(u32 x = 0;x < v2.size();x++){
						v2.at(x) *= layers[i]->activatorDerivative(intermediate[j].at(x));
					}
				}

				// v2 * transpose(v) (the result is a matrix)
				// we have a special function for this.

				totalGradient += Vector::crossNorm(v2,intermediate[i]); //tg = 3 4
				totalGradient *= learningRate; // TODO: adagrad here.
				layers[i]->updateMatrix(totalGradient); // adjuste the layer based on the average gradient.

				if(layers[i]->isBias()){
					v2 *= learningRate;
					layers[i]->updateBias(v2);
				}
			}
		}

	}

}
