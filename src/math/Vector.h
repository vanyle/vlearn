#pragma once

#include "utils/utils.h"
#include "Matrix.h"

namespace vio{

	struct Matrix; // forward declaration.
	
	// fixed size container meant for linear algebra
	struct Vector{
	private:
		u32 s;
		float * data;
	public:
		Vector(u32 size);
		Vector(const Vector& v);
		~Vector();

		Vector& operator=(const Vector& v);
		Vector& operator=(Vector&& v);

		void fill(float v);
		void fillRandom();

		float normSquared() const;
		float norm() const;

		u32 size() const;

		void print() const;

		float& at(u32 x);
		float get(u32 x) const;

		Vector& operator+=(const Vector& v);
		Vector& operator-=(const Vector& v);
		Vector& operator*=(const Vector& v); // element wise.
		Vector& operator*=(float v);
		Vector& operator/=(float v);

		static Vector add (const Vector& a,const Vector& b);
		static Vector sub(const Vector& a,const Vector& b);
		static float dot (const Vector& a,const Vector& b); // aᵀb
		static Matrix crossNorm(const Vector& a,const Vector& b); // abᵀ
	};

}