#include "vector.h"
#include "utils/utils.h"
#include "math.h"
#include <cstdio>

namespace vio{
	Vector::Vector(u32 size){
		this->s = size;
		this->data = new float[size];
	}
	Vector::Vector(const Vector& v){
		this->s = v.s;
		this->data = new float[v.s];
		for(u32 i = 0;i < v.s;i++){
			this->data[i] = v.data[i];
		}
	}
	Vector::~Vector(){
		delete this->data;
	}

	Vector& Vector::operator=(const Vector& v){
		this->~Vector();
		*this = Vector(v);
		return *this;
	}
	Vector& Vector::operator=(Vector&& v){
		this->s = v.s;
		this->data = v.data;
        // now remove the content of other so that it cannot be used anymore
        v.data = 0;
        v.s = 0;
        return *this;
    }

	u32 Vector::size() const{
		return s;
	}
	float& Vector::at(u32 i){
		vassert(i < s);
		return data[i];
	}
	float Vector::get(u32 i) const{
		vassert(i < s);
		return data[i];
	}
	float Vector::normSquared() const{
		float n = 0;
		for(u32 i = 0;i < s;i++){
			n += data[i] * data[i];
		}
		return n;
	}
	float Vector::norm() const{
		return std::sqrt(this->normSquared());
	}

	void Vector::fill(float v){
		for(u32 i = 0;i < s;i++){
			data[i] = v;
		}
	}
	void Vector::fillRandom(){
		for(u32 i = 0;i < s;i++){
			data[i] = randomFloat();
		}
	}

	Vector& Vector::operator+=(const Vector& v){
		vassert(v.size() == this->size());
		for(u32 i = 0;i < s;i++){
			data[i] += v.data[i];
		}
		return *this;
	}
	Vector& Vector::operator-=(const Vector& v){
		vassert(v.size() == this->size());
		for(u32 i = 0;i < s;i++){
			data[i] -= v.data[i];
		}
		return *this;
	}
	Vector& Vector::operator*=(const Vector& v){
		vassert(v.size() == this->size());
		for(u32 i = 0;i < s;i++){
			data[i] *= v.data[i];
		}
		return *this;
	}
	Vector& Vector::operator*=(float v){
		for(u32 i = 0;i < s;i++){
			data[i] *= v;
		}
		return *this;
	}
	Vector& Vector::operator/=(float v){
		for(u32 i = 0;i < s;i++){
			data[i] /= v;
		}
		return *this;
	}

	void Vector::print() const{
		printf("[ ");
		for(u32 i = 0;i < s;i++){
			printf("%.3f ",data[i]);
		}
		printf("]\n");
	}
	Vector Vector::add(const Vector& a,const Vector& b){
		vassert(a.s == b.s);
		Vector v(a.s);
		for(u32 i = 0;i < a.s;i++){
			v.data[i] = a.data[i] + b.data[i];
		}
		return v;
	}
	Vector Vector::sub(const Vector& a,const Vector& b){
		vassert(a.s == b.s);
		Vector v(a.s);
		for(u32 i = 0;i < a.s;i++){
			v.data[i] = a.data[i] - b.data[i];
		}
		return v;
	}

	float Vector::dot(const Vector& a,const Vector& b){
		vassert(a.s == b.s);
		float f = 0;
		for(u32 i = 0;i < a.s;i++){
			f += a.data[i] * b.data[i];
		}
		return f;
	}
	Matrix Vector::crossNorm(const Vector& a,const Vector& b){
		Matrix m(b.s,a.s);
		for(u32 y = 0;y < a.s;y++){
			for(u32 x = 0;x < b.s;x++){
				m.at(y,x) = a.data[y] * b.data[x];
			}
		}
		return m;
	}

}