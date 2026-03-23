#pragma once


glm::vec3 DoNothing(triangle* tri, int depth, glm::vec3 p, glm::vec3 dir) // use when testing shadow rays
{
    return vec3(0);
}

glm::vec3 Shade(triangle* tri, int depth, glm::vec3 p, glm::vec3 dir)
{
    vec3 col = vec3(0); // small ambient amount, light contribution in lecture notes, we also set a default light value in a lab-----------------
    
    glm::vec3 surface_col = tri->v1.col;
    glm::vec3 n = tri->v1.nor; // surface normal
    
    float i_amb = 0.1f;
    glm::vec3 ambient = i_amb * surface_col; // "use colour of any vertex for surface colour"

    col = ambient;

    float i_dff = dot(n, dir); // dot product between normal and light source
    glm::vec3 diffuse = i_dff * surface_col;
    
    // for each L in lights 
        // trace (ray to L)
        // there's only one light source, being the eye vector? --------
    
    // trace (ray to L);
    float t;
    trace(eye, dir, t, col, depth - 1, Shade);
        
    if( dot(tri->v1.nor, tri->v1.pos - p) ) // if no intersection (between ray and plane?) (if angle between these is not 0)
        col += diffuse; // colour += diffuse light contribution
    
    // trace reflections (below), refractions (not assessed) and shadows (?)
    if (depth) // secondary rays can be recursively generated up to a maximum depth
        // WHERE DO WE GET THE dir FOR THE NEW RAY? ------------------
        trace(eye, dir, t, col, depth - 1, Shade); // colour += trace(reflection ray)

    return col; // at which point the accumulated ray colour is used for the pixel
}

bool PointInTriangle(glm::vec3 pt, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3)
{
    
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
    glm::vec3 p = point;

    glm::vec3 N = glm::cross((tri->v2.pos - tri->v1.pos), (tri->v3.pos - tri->v1.pos)); // N = v12 cross v13
    tri->v1.nor; // use this instead? -------

    float t = glm::dot(p - o, N) / glm::dot(dir, N);
   
    return t; // return t (only float in this function) AND IN THE PSEUDOCODE
}

void trace(glm::vec3 o, glm::vec3 dir, float& t, glm::vec3& io_col, int depth, closest_hit p_hit)
{
    triangle closest_triangle;
    
    // for each tri in triangles, no list of tris, get tri from p_hit(?). closest_hit is a function pointer -----------------------
    float t_min = FLT_MAX, t_temp;
    
    for (triangle tri : tris) {
        
        // i. Compute if the ray intersects with a plane on which the triangle lies
        t_temp = RayTriangleIntersection(o, dir, &tri, tri.v1.pos); // t = RayTriangleIntersection(tri)
        
        // ii. Test if the point of intersection is inside the triangle. this might have to go elsewhere, e.g. in raytrace(), if PointInTriangle, then pass shade else doNothing ----------------------
                // makes sense to test if inside closest_triangle
        glm::vec3 q = o + dir * t_temp;
        bool inTriangle = PointInTriangle(q, tri.v1.pos, tri.v2.pos, tri.v3.pos);

        if ((t_temp < t_min) && inTriangle) {
            // save closest_triangle
                            // minimum value of t found?
            t = t_temp;
            t_min = t_temp;

            closest_triangle = tri;
        }
            
    }
    
    // return Shade(closest_triangle)
        // no return, just shade triangle or do nothing here
    p_hit(&closest_triangle, depth, closest_triangle.v1.pos, dir);
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
            
            glm::vec3 o = eye; // o = camera

            // colour of pixel = trace (ray)
            glm::vec3 col = bkgd; // by default
            float t, depth = max_recursion_depth; // does depth correspond to max_recursion_depth in donotedit
            
            // trace ray to get colour & shade closest triangle
            // trace(o, d, t, col, depth, Shade);
            // trace(o, d, t, col, depth, DoNothing);

            writeCol(col, pixel_x, pixel_y);
        }
    }
    std::clog << "\rFinish rendering.           \n";
}
