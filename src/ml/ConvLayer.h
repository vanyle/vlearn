/*
 * ConvLayer.h
 *
 *  Created on: 30 avr. 2021
 *      Author: vanyle
 */

#ifndef ML_CONVLAYER_H_
#define ML_CONVLAYER_H_

#include "Layer.h"
#include "math/Vector.h"
#include "math/Matrix.h"
#include "utils/utils.h"

namespace vio {

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
} /* namespace vio */

#endif /* ML_CONVLAYER_H_ */
