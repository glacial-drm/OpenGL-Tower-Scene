#pragma once

void ClearColourBuffer(float col[4])
{
	glm::vec3 c = {col[0], col[1], col[2]};

	for (int py = 0; py < PIXEL_H; py++) {
		for (int px = 0; px < PIXEL_W; px++) {
			writeColToDisplayBuffer(c, px, py);
		}
	}
}

void ClearDepthBuffer()
{
	for (int py = 0; py < PIXEL_H; py++) {
		for (int px = 0; px < PIXEL_W; px++) {

			depth_buffer[(py * PIXEL_W) + (px)] = FLT_MAX;
		}
	}
}


void ApplyTransformationMatrix(glm::mat4 T, vector<triangle>& tris)
{
	for (triangle &tri : tris) { // change to regular iteration if overhead too large
		// multiply matrices
		tri.v1.pos = T * tri.v1.pos;
		tri.v2.pos = T * tri.v2.pos;
		tri.v3.pos = T * tri.v3.pos;
	}
}

void NDC(glm::vec4& CLIP) {
	CLIP.x = CLIP.x / CLIP.w;
	CLIP.y = CLIP.y / CLIP.w;
	CLIP.z = CLIP.z / CLIP.w;
	CLIP.w = CLIP.w / CLIP.w;
}
void ApplyPerspectiveDivision(vector<triangle>& tris)
{
	// clip space to NDC space
	for (triangle& t : tris) {
		NDC(t.v1.pos);
		NDC(t.v2.pos);
		NDC(t.v3.pos);
	}

}

void SCREEN(glm::vec4& NDC, float w, float h) {
	
	NDC = { (NDC.x + 1.f) * (w / 2), (NDC.y + 1.f) * (h / 2), NDC.z, 0.f };
}
void ApplyViewportTransformation(int w, int h, vector<triangle>& tris)
{
	for (triangle& t : tris) {
		SCREEN(t.v1.pos, w, h);
		SCREEN(t.v2.pos, w, h);
		SCREEN(t.v3.pos, w, h);
	}

}

float line(glm::vec2 a, glm::vec2 b, glm::vec2 p) { // Implicit line equation
	return ((b.y - a.y) * p.x) + ((a.x - b.x) * p.y) + (b.x * a.y) - (a.x * b.y);
}
void ComputeBarycentricCoordinates(int px, int py, triangle t, float& alpha, float& beta, float& gamma)
{
	glm::vec2 p = {(float)px, (float)py};
	glm::vec2 a(t.v1.pos);
	glm::vec2 b(t.v2.pos);
	glm::vec2 c(t.v3.pos);

	alpha = line(b, c, p) / line(b, c, a);
	beta = line(a, c, p) / line(a, c, b);
	gamma = line(a, b, p) / line(a, b, c);

}


void ShadeFragment(triangle tri, float& alpha, float& beta, float& gamma, glm::vec3& col, float& depth)
{
	col = (alpha * tri.v1.col) + (beta * tri.v2.col) + (gamma * tri.v3.col);
	depth = (alpha * tri.v1.pos.z) + (beta * tri.v2.pos.z) + (gamma * tri.v3.pos.z);
}


void Rasterise(vector<triangle> tris)
{
	
	for (int py = 0; py < PIXEL_H; py++) // for each pixel y in Y dimension
	{
		float percf = (float)py / (float)PIXEL_H;
		int perci = percf * 100;
		std::clog << "\rScanlines done: " << perci << "%" << ' ' << std::flush;

		for (int px = 0; px < PIXEL_W; px++) // for each pixel x in X dimension
		{
			for (triangle& t : tris) { // for each triangle t

				// if pixel x,y is inside triangle t
				float alpha, beta, gamma;
				ComputeBarycentricCoordinates(px, py, t, alpha, beta, gamma); // calculate barycentric coords
				
				if (0 <= alpha && alpha <= 1 &&
					0 <= beta && beta <= 1 &&
					0 <= gamma && gamma <= 1) { // 2. Test if pixel is inside triangle
					
					// calculate the fragment colour
					
						// 3. Interpolating using vertex colours and shade values
					glm::vec3 col;
					float depth;
					
					ShadeFragment(t, alpha, beta, gamma, col, depth); // use calced col and depth here, depth buffer == z buffer ??? -------------0-------------

						// 4. Merging using the depth_buffer
					if (depth < depth_buffer[(py * PIXEL_W) + (px)]) {
						depth_buffer[(py * PIXEL_W) + (px)] = depth;
						
						// 5. Writing colours to the colour_buffer
						writeColToDisplayBuffer(col, px, PIXEL_H - py);
					}
					
					// writeColToDisplayBuffer(col, px, PIXEL_H - py);
				}					
			}
		}
	}
	std::clog << "\rFinish rendering.           \n";
}

void render(vector<triangle>& tris)
{
	// Clear colour and depth buffers
	float bgd[] = { 1.f, 1.f, 1.f, 1.f };
	ClearColourBuffer(bgd);
	ClearDepthBuffer();
	
	// 1. Vertex Transformation
			// model transformation
	glm::vec3 obj_pos = glm::vec3(0.1f, -2.5f, -6.f);
	glm::mat4 model = glm::mat4(1.f);
	model = glm::translate(model, obj_pos);
	ApplyTransformationMatrix(model, tris);

			// view transformation: no need here, no rotations etc

			// projection transformation
	glm::mat4 projection = glm::perspective(glm::radians(55.f), (float)PIXEL_W / (float)PIXEL_H, 0.1f, 10.f);
	ApplyTransformationMatrix(projection, tris);

	// 2. perspective division
	ApplyPerspectiveDivision(tris);

	// 3. viewport transformation
	ApplyViewportTransformation(PIXEL_W, PIXEL_H, tris);

	// 4. rasterisation
	Rasterise(tris);
}
