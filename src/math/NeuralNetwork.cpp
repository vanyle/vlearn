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
	void Layer::updateMatrix(const Matrix& m){}
	void Layer::updateBias(const Vector& v){}
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
	Vector DenseLayer::apply(const Vector& x){
		vassert(x.size() == inS);
		// y = relu(mx)
		Vector y = m.apply(x);
		y += b;
		for(u32 i = 0;i < y.size();i++){
			y.at(i) = activatorFn(y.get(i));
		}
		return y;
	}
	Vector DenseLayer::applyGradient(const Vector& in,const Vector& evaluationPosition,const Vector& unused){
		Vector r = m.applyTranspose(in);
		// Note that the evaluation position is the once after the layer has been applied.
		for(u32 x = 0;x < r.size();x++){
			r.at(x) *= this->activatorDerivativeFn(evaluationPosition.get(x));
		}
		return r;
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

	// Softmax layer implementation
	// Used as the last layer to convert everything to a probability distribution
	SoftMaxLayer::SoftMaxLayer(u32 inputSize) : Layer(inputSize,inputSize){
		this->bias = false;
		this->learnable = false;
	}
	SoftMaxLayer::~SoftMaxLayer(){

	}
	void SoftMaxLayer::print(){
		debug("Softmax Layer: %i",inS);
	}
	Vector SoftMaxLayer::apply(const Vector& x){
		return x.softmax();
	}
	Vector SoftMaxLayer::applyGradient(const Vector& in,const Vector& evaluationPosition,const Vector& previousEvaluationPosition){
		// dy_i/dz_j = -y_i * y_j.
		Matrix m(in.size(),in.size());
		for(u32 i = 0;i < in.size();i++){
			for(u32 j = 0;j < in.size();j++){
				if(i!=j){
					m.at(i,j) = -evaluationPosition.get(i) * evaluationPosition.get(j);
				}else{
					m.at(i,i) = (1-evaluationPosition.get(i))*evaluationPosition.get(i);
				}
			}
		} // compute jacobian of layer, than multiply it.
		return m.applyTranspose(in);
	}
	void SoftMaxLayer::updateMatrix(const Matrix& m){vassert(false);}
	void SoftMaxLayer::updateBias(const Vector& v){vassert(false);}

	// "Batchnorm" layer implementation
	// Used to normalize input so that it matches a normal disturbution (according to central limit theorem)
	BatchNormLayer::BatchNormLayer(u32 inputSize) : Layer(inputSize,inputSize){
		this->bias = false;
		this->learnable = false;
	}
	BatchNormLayer::~BatchNormLayer(){}
	Vector BatchNormLayer::apply(const Vector& x){
		float e = 0;
		for(u32 i = 0;i < x.size();i++){
			e += x.get(i);
		}
		e /= x.size();
		float v = 0;
		for(u32 i = 0;i < x.size();i++){
			float t = e - x.get(i);
			v += t*t;
		}
		v = std::sqrt(v);
		Vector r(x.size());
		for(u32 i = 0;i < x.size();i++){
			r.at(i) = (x.get(i) - e) / v; // standard score
		}

		return r;
	}
	Vector BatchNormLayer::applyGradient(const Vector& in,const Vector& evaluationPosition,const Vector& pep){
		float e = 0;
		for(u32 i = 0;i < pep.size();i++){
			e += pep.get(i);
		}
		e /= pep.size();
		float v = 0;
		for(u32 i = 0;i < pep.size();i++){
			float t = e - pep.get(i);
			v += t*t;
		}
		v = std::sqrt(v);
		Vector r(in);
		r /= v;
		return r;
	}
	void BatchNormLayer::print(){
		debug("BatchNorm Layer: %i",inS);
	}
	void BatchNormLayer::updateMatrix(const Matrix& m){vassert(false);}
	void BatchNormLayer::updateBias(const Vector& v){vassert(false);}

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
	void ConvLayer::randomInit(float dev,float mean){
		this->kernel.fillRandom(dev,mean);
	}
	void ConvLayer::setKernel(Matrix k){
		this->kernel = k;
	}
	Matrix& ConvLayer::getKernel(){
		return kernel;
	}
	void ConvLayer::print(){
		debug("Conv2D Layer %i x %i with kernel %i x %i",this->inputSize(),this->outputSize(),kernel.width(),kernel.height());
		kernel.print();
	}
	Vector ConvLayer::apply(const Vector& x){
		vassert(x.size() == inS);
		Vector y(inS / reduc / reduc);
		u32 c = 0;

		for(u32 i = 0;i < side_length;i += reduc){
			for(u32 j = 0;j < side_length;j += reduc){
				// apply conv matrix around x.at(i*size_length+j);
				float ytemp = 0;
				for(u32 ki = 0;ki < kernel.width();ki++){
					for(u32 kj = 0;kj < kernel.height();kj++){
						if(i+ki >= side_length || j+kj >= side_length) continue;
						ytemp += x.get((i+ki)*side_length+(j+kj)) * kernel.get(kj,ki);
					}
				}
				y.at(c) = ytemp;
				c+=1;
			}
		}

		for(u32 i = 0;i < y.size();i++){
			y.at(i) /= (y.at(i) < 0) ? 100 : 1;
		}
		return y;
	}
	Vector ConvLayer::applyGradient(const Vector& in,const Vector& evaluationPosition,const Vector& unused){
		vassert(in.size() == outputSize()); // reverse direction from apply.
		Vector res(inputSize());
		res.fill(0); // outputSize = inputSize / reduc / reduc

		const i32 ssl = side_length / reduc; // small side length,

		for(i32 i = 0;i < (i32)side_length;i++){
			for(i32 j = 0;j < (i32)side_length;j++){
				for(i32 si = max(0,((i-(i32)kernel.width()+(i32)reduc) / (i32)reduc));si < min(ssl,floor(i/reduc)+1);si++){
					for(i32 sj = max(0,((j-(i32)kernel.height()+(i32)reduc) / (i32)reduc));sj < min(ssl,floor(j/reduc)+1);sj++){
						const i32 kernelPosI = i-si*reduc;
						const i32 kernelPosJ = j-sj*reduc;

						vassert((kernelPosI>=0 && kernelPosI < (i32)kernel.width() && kernelPosJ >= 0 && kernelPosJ < (i32)kernel.height()));
						res.at(i*side_length+j) += kernel.get(kernelPosJ,kernelPosI) * in.get(si*ssl + sj);
					}
				}
			}
		}


		// apply gradient
		for(u32 i = 0;i < res.size();i++){
			res.at(i) *= (evaluationPosition.get(i)<0 ? 0.01 : 1);
		}

		return res;
	}
	void ConvLayer::updateMatrix(const Matrix& m){
		// m is of size inputSize * outputSize, this is bigger than the kernel.
		// we need to average the coefs of m in a repeated pattern to update the kernel
		// also note that most of the elements of m are 0.
		// this is inefficient (a sparse matrix would be better)
		// however, this would reduce genericity.
		Matrix kernelUpdate(kernel.width(),kernel.height());
		kernelUpdate.fill(0.);
		const u32 ssl = side_length / reduc;

		for(i32 i = 0;i < (i32)side_length;i ++){
			for(i32 j = 0;j < (i32)side_length;j++){
				for(i32 si = max(0,((i-(i32)kernel.width()+(i32)reduc) / (i32)reduc));si < min(ssl,floor(i/reduc)+1);si++){
					for(i32 sj = max(0,((j-(i32)kernel.height()+(i32)reduc) / (i32)reduc));sj < min(ssl,floor(j/reduc)+1);sj++){
						const i32 kernelPosJ = j-sj*reduc;
						const i32 kernelPosI = i-si*reduc;

						vassert((kernelPosI>=0 && kernelPosI < (i32)kernel.width() && kernelPosJ >= 0 && kernelPosJ < (i32)kernel.height()));

						kernelUpdate.at(kernelPosJ,kernelPosI) += m.get(si*ssl+sj,i*side_length+j);
					}
				}
			}
		}

		kernelUpdate *= ((float)reduc)/side_length;


		kernel -= kernelUpdate;
	}
	void ConvLayer::updateBias(const Vector& v){vassert(false);}

	// -----------------------------

	NeuralNetwork::NeuralNetwork(){
		this->errorFunction = L2errorFn;
		this->errorFunctionGradient = L2errorDerivativeFn;
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
					// could this leak memory ??
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
