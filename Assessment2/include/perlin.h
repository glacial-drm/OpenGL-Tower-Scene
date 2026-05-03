#include <iostream>
#include <math.h>
#include <glm/glm.hpp>

// generate random unit vector (magnitude 1)
glm::vec2 randomGradient(int ix, int iy) {
    
    // hash that takes a point and generates a random end-point 
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2;
    unsigned a = ix, b = iy;
    a *= 3284157443;

    b ^= a << s | a >> w - s;
    b *= 1911520717;

    a ^= b << s | b >> w - s;
    a *= 2048419325;
    float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]

    // Create the vector from the angle
    glm::vec2 v;
    v.x = sin(random);
    v.y = cos(random);

    return v;
}

// Computes the dot product of the distance and gradient (unit) vectors.
float dotGridGradient(int ix, int iy, float x, float y) {
    
    // Get gradient (unit vector) from integer coordinates
    glm::vec2 gradient = randomGradient(ix, iy);
    
    // Compute the distance vector (between a given corner and the chosen sample point)
    float dx = x - (float)ix;
    float dy = y - (float)iy;
    glm::vec2 distance = { dx, dy };

    // Compute the dot-product
    glm::dot(gradient, distance); // verif, then change to glm
    return (dx * gradient.x + dy * gradient.y);
}

float interpolate(float a0, float a1, float w) // interpolate the four corners (grid intersections) based on how close the sample point is to them
{
    // interpolates two corners at a time (twice for 4 corners), then called after on the results 
        // this uses cubic interpolation
    return (a1 - a0) * (3.0 - w * 2.0) * w * w + a0;
}


// Sample Perlin noise at coordinates x, y
float perlin_noise(float x, float y) {

    // Determine grid cell corner coordinates
    int x0 = (int)x;
    int y0 = (int)y;
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    // Compute Interpolation weights
    float sx = x - (float)x0;
    float sy = y - (float)y0;

    // Compute and interpolate top two corners
    float n0 = dotGridGradient(x0, y0, x, y);
    float n1 = dotGridGradient(x1, y0, x, y);
    float ix0 = interpolate(n0, n1, sx);

    // Compute and interpolate bottom two corners
    n0 = dotGridGradient(x0, y1, x, y);
    n1 = dotGridGradient(x1, y1, x, y);
    float ix1 = interpolate(n0, n1, sx);

    // Final step: interpolate between the two previously interpolated values, now in y
    float value = interpolate(ix0, ix1, sy);

    return value;
}

int perlin_vertices(int GRID_SIZE, glm::vec3 origin)
{
    // pixels should be the points within our grid that we want to sample
        // how much detail 
    const int windowWidth = 1920;
    const int windowHeight = 1080
    sf::Uint8* pixels = new sf::Uint8[windowWidth * windowHeight * 4];


    // larger the grid size, larger the features (sampled points are spread out more)
        // larger amount of points to sample from, yet we still sample the same total points
    const int GRID_SIZE = 400;


    for (int x = 0; x < windowWidth; x++)
    {
        for (int y = 0; y < windowHeight; y++)
        {
            int index = (y * windowWidth + x) * 4;


            float val = 0;

            float freq = 1;
            float amp = 1;

            for (int i = 0; i < 12; i++) // sample i points per pixel (for us it'll be just within the grid)
            {
                // MAIN ISSUE IS THAT WE NEED TO GET RID OF THE ADDITIVE PROPERTY ----------------------------------------
                    // instead of adding, we create a vertex
                        // we use the perlin noise for the normals, so we should already have the pos and colour
                        // each octave is a ____
                    // if it's procedurally generated, don't we have to generate the 
                val += perlin_noise(x * freq / GRID_SIZE, y * freq / GRID_SIZE) * amp;

                freq *= 2;
                amp /= 2;

            }

            // Contrast
            val *= 1.2;

            // Clipping
            if (val > 1.0f)
                val = 1.0f;
            else if (val < -1.0f)
                val = -1.0f;

            // Convert 1 to -1 into 255 to 0
            int color = (int)(((val + 1.0f) * 0.5f) * 255);

            // Set pixel color
            pixels[index] = color;
            pixels[index + 1] = color;
            pixels[index + 2] = color;
            pixels[index + 3] = 255;
        }
    }

    sf::Texture texture;
    sf::Sprite sprite;

    texture.create(windowWidth, windowHeight);

    texture.update(pixels);

    sprite.setTexture(texture);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(sprite);

        window.display();
    }

    return 0;
}


