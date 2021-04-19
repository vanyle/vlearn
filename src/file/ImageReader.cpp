/*
 * ImageReader.cpp
 *
 *  Created on: 19 avr. 2021
 *      Author: vanyle
 */

#include "file/ImageReader.h"

#define STBI_ASSERT(x) vassert(x)
#define STBI_WINDOWS_UTF8
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

ImageReader::ImageReader(std::string filename){
	data = stbi_load(filename.c_str(), &width, &height, &n, 3);
	if(data == 0){
		fail = true;
	}
}
bool ImageReader::isLoaded(){
	return !fail;
}
std::string ImageReader::loadFailCause(){
	return std::string(stbi_failure_reason());
}
u32 ImageReader::getWidth(){
	return width;
}
u32 ImageReader::getHeight(){
	return height;
}
bool ImageReader::isAlpha(){
	return n == 4;
}
unsigned char ImageReader::pixelAtPos(u32 x,u32 y,u32 channel){
	vassert(channel < (u32)n && x < (u32)width && y < (u32)height);
	return data[channel + n*(x + width*y)];
}

ImageReader::~ImageReader(){
	stbi_image_free(data);
}

