/*
 * ConvLayer.cpp
 *
 *  Created on: 30 avr. 2021
 *      Author: vanyle
 */

#include "ConvLayer.h"
#include "math/math.h"


namespace vio {

	// Convolution layer implementation
	// This only works on gray-scaled images.
	// You need to use a routing layer or a duplicating layer to separate the color channels to use this
	// on colored images.

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

} /* namespace vio */
