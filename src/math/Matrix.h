#pragma once
#include <cstdint>

#include "utils/utils.h"
#include "Vector.h"

namespace vio{

	struct Vector;

	struct Matrix{
	private:
		u32 w;
		u32 h;
		// only rows need to be close in memory for vector evaluation.
		// note that this means that multiplication can get very slow for large sizes.
		float * matData;
	public:
		Matrix() = delete;
		Matrix(u32 w,u32 h);
		Matrix(const Matrix& m); // copy.
		~Matrix();
		Matrix& operator=(const Matrix& m); // assignement
		Matrix& operator=(Matrix&& m); // move

		u32 width() const;
		u32 height() const;
		float get(u32 y,u32 x) const;
		float& at(u32 y,u32 x);

		void fill(float v);
		void fillRandom(float coef = 1);
		void transpose(); // assumes that w == h
		Matrix transpose() const; // assumes nothing, requires a copy.

		void print(); // debug.

		Matrix& operator+=(const Matrix& m);
		Matrix& operator-=(const Matrix& m);
		Matrix& operator*=(float v);
		
		Vector apply(const Vector& v) const; // returns this * v
		Vector applyTranspose(const Vector& v) const; // returns v * transpose(this) (but without copies of this)

		static Matrix mul(const Matrix& a,const Matrix& b);
	};

	// Add some data structure for GPU computation here.

}