#include <iostream>
#include <glm/glm.hpp>

#include <glm/gtc/noise.hpp>

int dim = 8;
int stride = 100;
int floatPerVert;

typedef struct {
	int dim; // grid size for a single chunk in both x and z, (0 to dim)
	int stride; // distance of a unit vector within the grid (1 -> n)

	glm::vec3 origin; // position to start rendering
	
	// specify the heights of coordinates within the chunk
		// 2D array because grid of points (of intersection) using "dim"
	float** heights;
} chunk;


chunk* createChunk(glm::vec3 origin, int height, bool isPerlin) {

	chunk* newChunk = new chunk;

	newChunk->dim = dim;
	newChunk->stride = stride;

	newChunk->origin = origin;
	newChunk->heights = new float*[newChunk->dim];

	for (int row = 0; row < newChunk->dim; row++) {
		
		newChunk->heights[row] = new float[newChunk->dim];

		for (int col = 0; col < newChunk->dim; col++) {
			
			newChunk->heights[row][col] = height; // randFloat; //glm::perlin(glm::vec2((float)row, (float)col));; // or use our implementation of perlin and talk about in R&D --------------------------------------

			if (isPerlin) {
				float randFloat = static_cast <float>(rand()) / static_cast <float>(RAND_MAX); // random between 0 and 1
				randFloat *= height;
				
				newChunk->heights[row][col] = randFloat;
			}

		}
	}

	return newChunk;
}

//chunk customChunk() { // why use a function to define this anyway, just hardcode the chunk
//	return;
//}

float textureChunk(float height) {

	float texArrPos = 1;

	if (height < 2) {
		texArrPos = 0;
	}
	else if (height > 5) {
		texArrPos = 2;
	}

	return texArrPos;
}

float* colourChunk(float height) {
	float col[3] = {0.f, 1.f, 0.f};

	if (height < 2) {
		col[0] = 0.f;
		col[1] = 0.f;
		col[2] = 1.f;
	}
	else if (height > 5) {
		col[0] = 1.f;
		col[1] = 0.f;
		col[2] = 0.f;
	}
	
	return col;
}

void writeVertex(float* verts, int row, int col, int& offset, chunk* chunk, float texX, float texY) {
	// THIS WOULD HAVE TO PASS THE VERTEX POSITION TO THE TEXTURE CHUNK FUNCTION --------------------------------
	
	// pos
	float height = chunk->origin.y + chunk->heights[row][col];
	
	verts[offset] = chunk->origin.x + (chunk->stride * row);
	verts[offset + 1] = height;
	verts[offset + 2] = chunk->origin.z + (chunk->stride * col);

	// col
	float* colour = colourChunk(height);

	verts[offset + 3] = colour[0];
	verts[offset + 4] = colour[1];
	verts[offset + 5] = colour[2];
		
	if (floatPerVert == 8) { // texture
		// texture has to repeat each square
		// then simply render the triangles from (0,0 to 1,1)
			// texture atlas would be aligning all 3 tex images, then applying a 0.3 or 0.6 modifier to x ----------------------------
				// lol no
		verts[offset + 6] = texX;
		verts[offset + 7] = texY;
	}
	else if (floatPerVert == 7) { // alpha
		verts[offset + 6] = 0.5f;
	}
	else { // 9 | normals
		//
		verts[offset + 3] = texX;
		verts[offset + 4] = texY;
		verts[offset + 5] = textureChunk(height); // texture position in array

		// ----------- original (normals)
		verts[offset + 6] = 0.f;
		verts[offset + 7] = 1.f;
		verts[offset + 8] = 0.f;
	}

	offset += floatPerVert; // also change the offset from 9 to 8 -----------------------------------
}

void writeTriangles(float* verts, int row, int col, int& offset, chunk* chunk) { // need to convert them to tris, that's why the vertices[] in lab squares have 6 per face

	// we need to draw 1 or 2 triangles per vertex
		// if row or col == 0
			// only draw triangle for which vertex is behind hypotenuse
		// if x or y == max
			// only draw triangle for which vertex is in front of hypotenuse
		// else
			// draw both triangles

	if (row != 0 && col != 0) { // if verts aren't on the west and south borders of the grid
		// draw triangle from
		// ___
		// \  |
		//  \ |
		//   \|

		writeVertex(verts, row, col, offset, chunk, 1, 1); // 1, 1
		writeVertex(verts, row - 1, col, offset, chunk, 0, 1); // 0, 1
		writeVertex(verts, row, col - 1, offset, chunk, 1, 0); // 1, 0
	}
	if (row != chunk->dim - 1 && col != chunk->dim - 1) { // if verts aren't north and east borders of the grid
		// draw triangle from
		// |\
		// | \
		// |__\

		writeVertex(verts, row, col, offset, chunk, 0, 0); // 0, 0
		writeVertex(verts, row + 1, col, offset, chunk, 1, 0); // 1, 0
		writeVertex(verts, row, col + 1, offset, chunk, 0, 1); // 0, 1
	}
}

float* chunkVertices(chunk* chunk, int vertFloatTotal) {
	// return list of chunk vertices to be rendered
	


	floatPerVert = vertFloatTotal;
	// *9 for pos, col, nor.
	// *10 adds alpha
	// *8 is for texture, without alpha and nor
	
	// (chunk->dim)^2 is number of intersection points. 
	// * floatPerVert based on above
	// *6 is:
		// *3 (triangle = 3 verts)
		// *2 (two triangles per vert)
	float* verts = new float[chunk->dim * chunk->dim * floatPerVert * 6];

	int offset = 0;
	for (int row = 0; row < chunk->dim; row++) { // corresponds to x
		for (int col = 0; col < chunk->dim; col++) { // corresponds to z
			
			// for each vertex (point of intersection)
			writeTriangles(verts, row, col, offset, chunk);

		}
	}

	return verts;
}

void connectTris(chunk* target, chunk* source, bool west_south) {

	// bool west_south
		// west = 0
		// south = 1

	// connect target west and/or south to source east/north
	for (int row = 0; row < target->dim; row++) {
		
		for (int col = 0; col < target->dim; col++) {
			
			// get 0th row/column of target, apply connections
			if (row == 0 && west_south) { target->heights[row][col] = source->heights[target->dim - 1][col]; }
			
			if (col == 0 && !west_south) { target->heights[row][col] = source->heights[row][target->dim - 1]; }

		}
	}
}