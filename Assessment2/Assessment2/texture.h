#pragma once

#include <iostream>
#include "stb_image.h"

GLuint setup_texture(const char* filename)
{
	// Enable Textures
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	// Generate OpenGL Texture object
	GLuint texObject;
	glGenTextures(1, &texObject);
	glBindTexture(GL_TEXTURE_2D, texObject);

	// Set params on how OpenGL draws the textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // enable Bi-linear filtering - texels being larger than fragments with magnification no longer results in pixelation
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable trilinear filtering (mipmaps?)

	// Load image file
	int w, h, chan;
	stbi_set_flip_vertically_on_load(true); // flip y axis of image (it was rendered upside down)
	unsigned char* pxls = stbi_load(filename, &w, &h, &chan, 0);
		// stbi_load params
			// Load image file at "filename" location
			// Loaded file is "x" by "y"
			// channels_in_file is loaded into chan, 3 for RGB and 4 for RGBA
			// last param is desired channels
	if (pxls)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, pxls);
		// params
			// target
			// level - used later on
			// internalformat - number of colour components
			// width, height - dimensions of the image
			// border
			// format 
			// type
			// data - colour data that was just loaded
	}
	glGenerateMipmap(GL_TEXTURE_2D);
	delete[] pxls; // delete colour data as no longer needed after being loaded to OpenGL

	// Disable textures
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	return texObject;
}

GLuint setup_mipmaps(const char* filename[], int n) // does the same as setup_texture() but for each of the n filenames
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // enable trilinear filtering for minification filter

	int w[16], h[16], chan[16];
	unsigned char* pxls[16];
	stbi_set_flip_vertically_on_load(true);
	for (int c = 0; c < n; c++) // loop for each of the fienames
	{
		pxls[c] = stbi_load(filename[c], &w[c], &h[c], &chan[c], 0);
		if (pxls[c])
		{
			// level is changed to c (compared to setup_texure()) to make use of mipmaps
			glTexImage2D(GL_TEXTURE_2D, c, GL_RGB, w[c], h[c], 0, GL_RGB, GL_UNSIGNED_BYTE, pxls[c]);
		}

		delete pxls[c];
	}
	return 0;
}