#pragma once

#include "bitmap.h"

struct ShadowStruct
{
	// Members used to access the depth map for shadow mapping
	unsigned int FBO;
	unsigned int Texture;
};


ShadowStruct setup_shadowmap(int w, int h)
{
	ShadowStruct shadow;

	
	glGenFramebuffers(1, &shadow.FBO); // generate n framebuffer objects, store in ids arr
	glBindFramebuffer(GL_FRAMEBUFFER, shadow.FBO); // bind framebuffer to target so we can make changes to framebuffer, binding shadow.FBO
	glGenTextures(1, &shadow.Texture); // generate n textures and store in textures arr
	glBindTexture(GL_TEXTURE_2D, shadow.Texture); // bind texture to target so we can make changes to texture, binding shadow.Texture
	
	// specifies 2d texture image for the depth map
		// target = GL_TEXTURE_2D
		// level = 0 as there are no mipmaps in shadow maps
		// internalFormat = GL_DEPTH_COMPONENT as we are only using a depth component
		// width and height are sent to passed args
		// border = 0
		// format = GL_DEPTH_COMPONENT
		// type = GL_FLOAT as the depth values from z-buffer are floats
		// data = null
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL); 
	
	// similar to the ones from lab6b
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow.FBO);
	
	// attaches a texture image to a framebuffer obj
		// target = GL_FRAMEBUFFER
		// attachment = GL_DEPTH_ATTACHMENT so the values from depth buffer are storedi n the texture image
		// texttarget = GL_TEXTURE_2D
		// texture is set to member object specifies the texture object
		// level = 0
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow.Texture, 0);
	
	glDrawBuffer(GL_NONE); // set to GL_NONE so no colour buffers are written during the render pass
	glReadBuffer(GL_NONE); // set to GL_NONE so no colour buffers are read during the render pass
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	return shadow;
}

void saveShadowMapToBitmap(unsigned int Texture, int w, int h)
{
	float* pixelBuffer = (float*)malloc(sizeof(float) * w * h);// [] ;
	glBindTexture(GL_TEXTURE_2D, Texture);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, pixelBuffer);

	char* charBuffer = (char*)malloc(sizeof(unsigned char) * w * h * 3);

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			charBuffer[(y * w * 3) + (x * 3) + 0] = pixelBuffer[(y * w) + x] * 255;
			charBuffer[(y * w * 3) + (x * 3) + 1] = pixelBuffer[(y * w) + x] * 255;
			charBuffer[(y * w * 3) + (x * 3) + 2] = pixelBuffer[(y * w) + x] * 255;
		}

	}

	BITMAPINFOHEADER infoHdr;
	infoHdr.biSize = 40;
	infoHdr.biWidth = w;
	infoHdr.biHeight = h;
	infoHdr.biPlanes = 1;
	infoHdr.biBitCount = 24;
	infoHdr.biCompression = 0;
	infoHdr.biSizeImage = sizeof(unsigned char) * w * h * 3;
	infoHdr.biXPelsPerMeter = 0;
	infoHdr.biYPelsPerMeter = 0;
	infoHdr.biClrUsed = 0;
	infoHdr.biClrImportant = 0;

	BITMAPFILEHEADER fileHdr;
	fileHdr.bfType = 19778;
	fileHdr.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (sizeof(unsigned char) * w * h * 3);
	fileHdr.bfReserved1 = 0;
	fileHdr.bfReserved2 = 0;
	fileHdr.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	savebitmap("shadowMap.bmp", (unsigned char*)charBuffer, &infoHdr, &fileHdr);

	free(charBuffer);
	free(pixelBuffer);
}