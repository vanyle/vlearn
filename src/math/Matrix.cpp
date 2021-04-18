#include "Matrix.h"
#include "math.h"
#include "../utils/utils.h"
#include <cstdio>

namespace vio{

	Matrix::Matrix(u32 w,u32 h){
		this->w = w;
		this->h = h;
		// allocate.
		// m[y][x]
		this->matData = new float[h*w];
	}
	Matrix::Matrix(const Matrix& m){
		this->w = m.w;
		this->h = m.h;
		// allocate.
		// m[y][x]
		this->matData = new float[h*w];
		u32 s = w*h;
		for(u32 i = 0;i < s;i++){
			this->matData[i] = m.matData[i];
		}
	}
	Matrix::~Matrix(){
		delete this->matData;
	}
	Matrix& Matrix::operator=(const Matrix& m){
		this->~Matrix();
		*this = Matrix(m);
		return *this;
	}
	Matrix& Matrix::operator=(Matrix&& m){
		this->matData = m.matData;
		this->w = m.w;
		this->h = m.h;
		m.matData = 0;
		m.w = 0;
		m.h = 0;
		return *this;
	}
	u32 Matrix::width() const{
		return this->w;
	}
	u32 Matrix::height() const{
		return this->h;
	}
	void Matrix::transpose(){
		vassert(h == w);
		for(u32 y = 0;y < h;y++){
			for(u32 x = y+1;x < w;x++){
				// swap
				float temp = this->get(x,y);
				this->at(x,y) = this->get(y,x);
				this->at(y,x) = temp;
			}
		}
	}
	Matrix Matrix::transpose() const{
		Matrix transposed(h,w);
		u32 i = 0;
		for(u32 y = 0;y < h;y++){
			for(u32 x = 0;x < w;x++){
				transposed.at(x,y) = matData[i];
				i+=1;
			}
		}
		return transposed;
	}
	float& Matrix::at(u32 y,u32 x){
		vassert(y < h && x < w);
		return this->matData[y*w+x];
	}
	float Matrix::get(u32 y,u32 x) const{
		vassert(y < h && x < w);
		return this->matData[y*w+x];		
	} 
	void Matrix::fill(float v){
		u32 s = w*h;
		for(u32 i = 0;i < s;i++){
			this->matData[i] = v;
		}
	}
	void Matrix::fillRandom(float coef){
		u32 s = w*h;
		for(u32 i = 0;i < s;i++){
			this->matData[i] = randomFloat() * coef;
		}
	}
	void Matrix::print(){
		printf("[");
		for(u32 y = 0;y < h;y++){
			printf("[");
			for(u32 x = 0;x < w;x++){
				printf("%.3f ",this->get(y,x));
			}
			printf("]");
			if(y != h-1){ printf("\n"); }
		}
		printf("]\n");
	}


	Matrix& Matrix::operator+=(const Matrix& m){
		vassert(w == m.w && h == m.h);
		u32 s = w*h;
		for(u32 i = 0;i < s;i++){
			this->matData[i] += m.matData[i];
		}
		return *this;
	}
	Matrix& Matrix::operator-=(const Matrix& m){
		vassert(w == m.w && h == m.h);
		u32 s = w*h;
		for(u32 i = 0;i < s;i++){
			this->matData[i] -= m.matData[i];
		}
		return *this;
	}
	Matrix& Matrix::operator*=(float v){
		u32 s = w*h;
		for(u32 i = 0;i < s;i++){
			this->matData[i] *= v;
		}
		return *this;
	}
	Vector Matrix::apply(const Vector& v) const{
		vassert(v.size() == this->w);
		Vector res(this->h);
		res.fill(0);

		for(u32 y = 0;y < h;y++){
			for(u32 x = 0;x < w;x++){
				res.at(y) += this->get(y,x) * v.get(x);
			}
		}
		return res;
	}
	Vector Matrix::applyTranspose(const Vector& v) const{
		vassert(v.size() == this->h);
		Vector res(this->w);
		res.fill(0);

		for(u32 y = 0;y < h;y++){
			for(u32 x = 0;x < w;x++){
				res.at(x) += this->get(y,x) * v.get(y);
			}
		}
		return res;
	}

	Matrix Matrix::mul(const Matrix& a,const Matrix& b){
		vassert(a.w == b.h);
		Matrix res(b.w,a.h);
		for(u32 i = 0;i < res.h;i++){
			for(u32 j = 0;j < res.w;j++){
				res.at(i,j) = 0;
				for(u32 k = 0;k < b.w;k++){
					res.at(i,j) += a.get(i,k) * b.get(k,j);
				}
			}
		}

		return res;
	}


}