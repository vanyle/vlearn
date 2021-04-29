/*
 * ImageReader.h
 *
 *  Created on: 19 avr. 2021
 *      Author: vanyle
 */

#pragma once

#include <string>
#include "utils/utils.h"
/**
A class to read images.

Example: display the size of an image and the content of the top-left pixel.
@code
ImageReader ir(getExecutableFolderPath() + "/example2.png");

debug("Size: w,h,alpha %i %i %i",ir.getWidth(),ir.getHeight(),ir.isAlpha());
debug("Pixel value (RGB): %i %i %i",ir.pixelAtPos(0,0,0),ir.pixelAtPos(0,0,1),ir.pixelAtPos(0,0,2));
@endcode
 */
class ImageReader {
private:
	unsigned char *data = 0;
	int width = 0;
	int height = 0;
	int n = 0;
	bool fail = false;
public:
	u32 getWidth();
	u32 getHeight();
	bool isAlpha();
	bool isLoaded();
	std::string loadFailCause();
	void save(std::string filename);

	void setPixelAtPos(u32 x,u32 y,u32 color_channel,char pvalue);

	unsigned char pixelAtPos(u32 x,u32 y,u32 color_channel);

	ImageReader(std::string filename);
	~ImageReader();
};
