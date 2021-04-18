
#include "BufferedDataStream.h"
#include "utils/utils.h"
#include "math/math.h"

namespace vio{

BufferedDataStream::BufferedDataStream(unsigned int capacity){
	this->data = new char[capacity];
	this->capacity = capacity;
}
BufferedDataStream::~BufferedDataStream(){
	delete[] this->data;
}
unsigned int BufferedDataStream::spaceAvailable(){
	return this->capacity - this->length;
}
unsigned int BufferedDataStream::dataStoredSize(){
	return this->length;
}
void BufferedDataStream::write(void * input,unsigned int size){ // write input to buffer
	if(isClosed){
		return;
	}
	while(size > this->spaceAvailable()){
		if(isClosed){
			return;
		}
		this->dataMutex.lock();
	}
	for(uint i = 0;i < size;i++){
		uint idx = mod(((int)this->start)-1-i,(int)this->capacity);
		this->data[idx] = ((char*)input)[i];
	}
	this->length += size;
	this->start = mod(((int)this->start) - size,(int)this->capacity);

}
void BufferedDataStream::close(){
	this->isClosed = true;
	this->dataMutex.unlock();
}
unsigned int BufferedDataStream::read(void * output,unsigned int size){
	if(isClosed){
		return 0;
	}
	uint i;
	for(i = 0;i < size;i++){
		if(i >= this->length) break;
		uint idx = mod(this->start + this->length - 1 - i,(int)this->capacity);
		((char*)(output))[i] = this->data[idx];
	}
	this->length -= i;
	this->dataMutex.unlock();
	return i;
}

}