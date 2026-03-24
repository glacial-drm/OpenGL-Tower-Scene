#pragma once


glm::vec3 DoNothing(triangle* tri, int depth, glm::vec3 p, glm::vec3 dir) // use when testing shadow rays
{
    return vec3(0); // bkgd;// (?)
}

glm::vec3 Shade(triangle* tri, int depth, glm::vec3 p, glm::vec3 dir)
{
    vec3 col = vec3(0);
    
    glm::vec3 surface_col = tri->v1.col; // "use colour of any vertex for surface colour"
    glm::vec3 n = glm::normalize(tri->v1.nor); // surface normal, normalised
    glm::vec3 o = p; // point of intersection (origin changes with different rays)
    glm::vec3 l = glm::normalize(light_pos - o); // vector l, from p to light source, normalised
   
    float i_amb = 0.1f;
    glm::vec3 ambient = i_amb * surface_col;

    float i_diff = glm::dot(n, l); // dot product between normal and light source
    glm::vec3 diffuse = i_diff * surface_col;
    
    col = ambient; // col = small ambient amount
    
    // trace (ray to L, from point), calc if the point is in shadow
        // shadows (not recursive so 0 depth)
        // DoNothing is for testing shadows
    float t_sha = 0.f;
    trace(o, l, t_sha, col, 0, DoNothing);
        
    if(t_sha == FLT_MAX) // if no intersection (between ray and another triangle)
        col += diffuse; // colour += diffuse light contribution
    
    if (depth) { // reflections can be recursively generated up to a maximum depth (recursion is here)
        
        // we reflect it off the normal
        float t_ref = 0.f;
        glm::vec3 refl = glm::reflect(dir, n);
        trace(o, refl, t_ref, col, depth - 1, Shade); // colour += trace(reflection ray)
    }

    // WE REALLY REALLY REALLY NEED TO VERIFY THIS COLOUR HERE --------------
    return col; // at which point the accumulated ray colour is used for the pixel
}

bool PointInTriangle(glm::vec3 pt, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)
{
    // take calced point on plane (triangle is also on plane)
        // check if point is in triangle
    glm::vec3 q = pt;

    float d1 = glm::dot(glm::cross((v2 - v1), (q - v1)), glm::cross((v2 - v1), (v3 - v1)));
    float d2 = glm::dot(glm::cross((v3 - v2), (q - v2)), glm::cross((v3 - v2), (v1 - v2)));
    float d3 = glm::dot(glm::cross((v1 - v3), (q - v3)), glm::cross((v1 - v3), (v2 - v3)));
    
    if (d1 > 0 && d2 > 0 && d3 > 0) {
        return true;
    }
    return false;
}

float RayTriangleIntersection(glm::vec3 o, glm::vec3 dir, triangle* tri, glm::vec3& point)
{
    // surely this is ray-plane moreso intersection

    glm::vec3 p = point; // point on the plane

    //glm::vec3 N = glm::cross((tri->v2.pos - tri->v1.pos), (tri->v3.pos - tri->v1.pos)); // N = v12 cross v13
    glm::vec3 N = tri->v1.nor; // could use this instead (if too computationally expensive)

    float t = glm::dot(p - o, N) / glm::dot(dir, N);
   
    return t; // return t, length to plane from ray origin
}

void trace(glm::vec3 o, glm::vec3 dir, float& t, glm::vec3& io_col, int depth, closest_hit p_hit)
{
    triangle* closest_triangle = new triangle;
    
    // for each tri in triangles
    float t_min = FLT_MAX, t_temp;
    glm::vec3 q;
    bool inTriangle;
    for (triangle tri : tris) { // get closest triangle
        
        // i. Compute if the ray intersects with a plane on which the triangle lies
        t_temp = RayTriangleIntersection(o, dir, &tri, tri.v1.pos); // t = RayTriangleIntersection(tri)
        
        // ii. Test if the point of intersection is inside the triangle. this might have to go elsewhere, e.g. in raytrace(), if PointInTriangle, then pass shade else doNothing ----------------------
                // makes sense to test if inside closest_triangle
        q = o + (dir * t_temp);
        inTriangle = PointInTriangle(q, tri.v1.pos, tri.v2.pos, tri.v3.pos);

        if ((t_temp < t_min) && inTriangle) {
            // save closest_triangle
                // minimum value of t found (across all iterations)
            t = t_temp;
            t_min = t_temp;

            closest_triangle = &tri; // changed after changing closest_triangle to pointer ----------------
        }
            
    }
    
    // return Shade(closest_triangle)
        // CURRENT ISSUE IS THAT OUR COLOURS ARE MISSING/OVERWRITTEN, WE ARE LITERALLY DOING NOTHING IN THE BELOW IF STATEMENT -------------------
        // clearly this is where we discover the triangle to shade (we could then literally just shade it but nah raytracing :p)    

            // we then calc whether or not its in shadow
                // if in shadow, add diffuse to ambience and do nothing
                // if not in shadow, set colour equal to triangle colour

            // we ALSO calc the reflection ray (so do another trace call containing Shade)
                // what does finding the closest triangle do here
                    // doing the exact same thing as calcing shadow, but for a recursive case, simple
    
    if (t_min != FLT_MAX) // if there is a closest triangle (NEED OTHERWISE NULL)
        // maybe we don't overwrite the colour, the return value is just for the end otherwise we aren't really +=ing -----------------------------------------------
        io_col += p_hit(closest_triangle, depth, q, dir);
        // io_col = vec3(255, 255, 255); // this does nothing so something is wrong
    else // if we don't find a closest triangle (default t_min)
        // surely the implication here is that we either set the original colour, not the bkgd(?)
    //    io_col = closest_triangle->v1.col; // we are checking from the ray, not the triangle (?) | we also know if we find a triangle from a ray we then calc the shadow and reflec rays (using Shade like above) these values are only tiny as they're in the range of 0-1
        t = FLT_MAX; // set as otherwise we ignore t entirely
        io_col += bkgd;  // WE ONLY SET TO BACKGROUND IF THERE IS NOT AN INTERSECTION
}

vec3 GetRayDirection(float px, float py, int W, int H, float aspect_ratio, float fov)
{
    glm::vec3 direction;
    glm::vec3 R = { 1.f, 0.f, 0.f };
    glm::vec3 U = { 0.f, 1.f, 0.f };
    glm::vec3 F = { 0.f, 0.f, -1.f };

    float f = tan(radians(fov) / 2);
    float a = aspect_ratio;
    R = (float)(a * f * ((2 * (px + 0.5) / W) - 1)) * R;
    U = (float)(f * ((2 * (py + 0.5)/ H) - 1)) * U;

    direction = R + U + F;
    return direction;
}

void raytrace()
{
    for (int pixel_y = 0; pixel_y < PIXEL_H; ++pixel_y) // for each pixel_y in PIXEL_H
    {
        float percf = (float)pixel_y / (float)PIXEL_H;
        int perci = percf * 100;
        std::clog << "\rScanlines done: " << perci << "%" << ' ' << std::flush;

        for (int pixel_x = 0; pixel_x < PIXEL_W; ++pixel_x) // for each pixel_x in PIXEL_W
        {
            // ray = GetRayDirection for pixel_x, pixel_y
            float aspect_ratio = (float)PIXEL_W / (float)PIXEL_H;
            glm::vec3 d = GetRayDirection(pixel_x, pixel_y, PIXEL_W, PIXEL_H, aspect_ratio, 90.f);
            d = glm::normalize(d); // normalise d to make it a direction vector
            
            glm::vec3 o = eye; // o = camera, this value is fine stop checking it (look at the x y and z specifically)

            // colour of pixel = trace (ray)
            glm::vec3 col = glm::vec3(0.f); // setting col to 0.f made weird black shape
            float t, depth = max_recursion_depth;
            
            // trace ray to get colour & shade closest triangle
            trace(o, d, t, col, depth, Shade); // Shade is used to calc colour, not DoNothing

            writeCol(col, pixel_x, pixel_y);
        }
    }
    std::clog << "\rFinish rendering.           \n";
}
