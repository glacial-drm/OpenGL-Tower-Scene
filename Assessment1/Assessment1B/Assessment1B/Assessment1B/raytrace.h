#pragma once


glm::vec3 DoNothing(triangle* tri, int depth, glm::vec3 p, glm::vec3 dir) // use when testing shadow rays
{
    return vec3(0);
}

glm::vec3 Shade(triangle* tri, int depth, glm::vec3 p, glm::vec3 dir)
{
    vec3 col = vec3(0);
    
    glm::vec3 surface_col = tri->v1.col; // use colour of any vertex for surface colour

    glm::vec3 n = glm::normalize(tri->v1.nor); // surface normal, normalised
    glm::vec3 o = p; // point of intersection (origin changes with different rays)
    glm::vec3 l = glm::normalize(light_pos - o); // vector l, from p to light source, normalised
   
    float i_amb = 0.1f;
    glm::vec3 ambient = i_amb * surface_col;

    float i_diff = glm::dot(n, l); // dot product between normal and light source, both normalised
    if (i_diff < 0) { // we ignore light 
        i_diff = 0;
    }
    glm::vec3 diffuse = i_diff * surface_col;
    
    col = ambient; // col = small ambient amount
    
    // trace (ray to L, from point), calc if the point is in shadow (using DoNothing)
        // shadows (not recursive so 0 depth)
    float t = 0.f;
    // trace(o, l, t, col, 0, DoNothing);
        
    if (t == FLT_MAX) { // if no intersection (between ray and another triangle)
        col += diffuse; // colour += diffuse light contribution
    }

    if (depth && tri->reflect) { // reflections can be recursively generated up to a maximum depth
        
        // we reflect it off the normal
        t = 0.f;
        glm::vec3 refl = glm::reflect(dir, n);
        // trace(o, refl, t, col, depth - 1, Shade); // colour += trace(reflection ray)
    }

    return col; // at which point the accumulated ray colour is used for the pixel
}

bool PointInTriangle(glm::vec3 pt, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)
{
    // take calced point on plane (triangle is also on plane)
        // check if point is in triangle
    glm::vec3 q = pt;

    // Apparently no need to normalise them, realistically all we're concerned about is whether they are positive or not
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
    // this is ray-plane intersection, we calc the actual point later

    glm::vec3 p = point; // known point on the plane (from triangle)
    glm::vec3 d = dir; // THIS HAS ALREADY BEEN NORMALISED
    glm::vec3 N = glm::normalize(glm::cross((tri->v2.pos - tri->v1.pos), (tri->v3.pos - tri->v1.pos))); // N = v12 cross v13, normalised to make unit vector

    float t = glm::dot(p - o, N) / glm::dot(d, N);
   
    return t;
}

void trace(glm::vec3 o, glm::vec3 dir, float& t, glm::vec3& io_col, int depth, closest_hit p_hit)
{
    triangle closest_triangle; // no triangle* as it just points to tri which changes value
    
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

            closest_triangle = tri;
        }
    }
    
    // return Shade(closest_triangle)
        // clearly this is where we discover the triangle to shade (we could then literally just shade it but nah raytracing :p)    

            // we then calc whether or not its in shadow
                // if in shadow, add diffuse to ambience and DoNothing
                // if not in shadow, set colour equal to triangle colour

            // we ALSO calc the reflection ray (so do another trace call containing Shade)
                // what does finding the closest triangle do here
                    // doing the exact same thing as calcing shadow, but for a recursive case, simple
    
    if (t_min != FLT_MAX) { // if there is a closest triangle (NEED OTHERWISE NULL)
        io_col += p_hit(&closest_triangle, depth, q, dir); // has to be plus equals otherwise overwriting colour
    }
    else { // if we don't find a closest triangle (default t_min)
        t = FLT_MAX; // set as otherwise the value of t doesn't change
        io_col += bkgd;
    }
}

vec3 GetRayDirection(float px, float py, int W, int H, float aspect_ratio, float fov)
{
    glm::vec3 direction;
    glm::vec3 R = { 1.f, 0.f, 0.f };
    glm::vec3 U = { 0.f, -1.f, 0.f };
    glm::vec3 F = { 0.f, 0.f, -1.f };

    float f = tan(fov / 2);
    float a = aspect_ratio;
    float n = (a * f * ((2 * (px + 0.5) / W) - 1));
    float m = (f * ((2 * (py + 0.5) / H) - 1));
    R = n * R;
    U = m * U;

    direction = R + U + F;
    return glm::normalize(direction);
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
            float fov = glm::radians(90.f);
            glm::vec3 ray = GetRayDirection(pixel_x, pixel_y, PIXEL_W, PIXEL_H, aspect_ratio, fov);// normalise d to make it a direction vector

            // colour of pixel = trace (ray)
            // trace ray to get colour & shade closest triangle
                // eye = o = camera
            glm::vec3 o = eye;
            glm::vec3 col = glm::vec3(0.f);
            float t, depth = max_recursion_depth;
            trace(o, ray, t, col, depth, Shade); // Shade is used to calc colour, not DoNothing

            writeCol(col, pixel_x, pixel_y);
        }
    }
    std::clog << "\rFinish rendering.           \n";
}
