#pragma once

#include "utils/utils.h"
#include <cmath>
/**
	math.h provide math, number related function not found in <cmath>

	This includes:
	- fast randomness (seedable)
	- perlin noise
	- modulo operations
	- nChoosek
	- ...
*/

namespace vio{

	// defined here because should not have to include algorithm to have access to max.
	#define max(a,b) \
	   ({ __typeof__ (a) _a = (a); \
		   __typeof__ (b) _b = (b); \
		 _a > _b ? _a : _b; })
	#define min(a,b) \
	   ({ __typeof__ (a) _a = (a); \
		   __typeof__ (b) _b = (b); \
		 _a <= _b ? _a : _b; })

	void seed();
	void seed(u32 value);
	void seed(u32 value1,u32 value2); // use this if you want more entropy.

	float randomFloat();
	float random(); // alias
	double randomDouble();
	u32 randomU32(const u32 max); // max included
	i32 randomI32(const i32 min,const i32 max); // max and min included

	double randAtInt(i32 x,i32 y,i32 z);

	inline float mod(float base, float div) { // modulus for floating numbers
		float fractPart = modff(base / div, nullptr); // nullptr is usually used to retrieve the integer part of base / div (pointer to float)
		if (fractPart <= 0) fractPart += 1;
		return fractPart * div;
	}
	inline i32 mod(i32 base, i32 div) {
		i32 m = base % div;
		if (m < 0) return m + div;
		return m;
	}

	u32 nChoosek(u32 n, u32 k);
	i32 modPow(i32 base,i32 exponent,i32 modulus);

	double perlin(double x, double y, double z);
	double isoperlin(double x,double y,double z);

}
