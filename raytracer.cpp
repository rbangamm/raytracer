#include <cstdlib> 
#include <cstdio> 
#include <cmath> 
#include <fstream> 
#include <vector> 
#include <set>
#include <iostream> 
#include <cassert>
#include "shapes.h"


//Controls max recursion depth
#define MAX_RAY_DEPTH 20

float mix(const float &a, const float &b, const float &mix)
{
    return b * mix + a * (1 - mix);
}

// Checks if any rays intersect any of the shapes in the scene.
// If the ray does intersect an object, we find the intersection
// point as well as the normal vector at the IP and then shade the
// point using the given info.
// Shading is done by taking into account the surface properties
// (i.e. is it transparent? reflective? diffuse?).
// Returns a color for the ray.
// This color will either be the color of the object at the intersection
// point or the background color depending on whether there was an
// intersection or not.
Vec3f trace( 
    const Vec3f &rayorig, 
    const Vec3f &raydir, 
    const std::vector<Sphere> &spheres, 
    const int &depth) 
{ 
    //if (raydir.length() != 1) std::cerr << "Error " << raydir << std::endl;
    float tnear = INFINITY; 
    const Sphere* sphere = NULL; 
    // find intersection of this ray with the sphere in the scene
    for (unsigned i = 0; i < spheres.size(); ++i) { 
        float t0 = INFINITY, t1 = INFINITY; 
        if (spheres[i].intersect(rayorig, raydir, t0, t1)) { 
            if (t0 < 0) t0 = t1; 
            if (t0 < tnear) { 
                tnear = t0; 
                sphere = &spheres[i]; 
            } 
        } 
    } 
    // if there's no intersection return black or background color
    if (!sphere) return Vec3f(2); 
    Vec3f surfaceColor = 0; // color of the ray/surfaceof the object intersected by the ray 
    Vec3f phit = rayorig + raydir * tnear; // point of intersection 
    Vec3f nhit = phit - sphere->center; // normal at the intersection point 
    nhit.normalize(); // normalize normal direction 
    // If the normal and the view direction are not opposite to each other
    // reverse the normal direction. That also means we are inside the sphere so set
    // the inside bool to true. Finally reverse the sign of IdotN which we want
    // positive.
    float bias = 1e-4; // add some bias to the point from which we will be tracing 
    bool inside = false; 
    if (raydir.dot(nhit) > 0) nhit = -nhit, inside = true; 
    if ((sphere->transparency > 0 || sphere->reflection > 0) && depth < MAX_RAY_DEPTH) { 
        float facingratio = -raydir.dot(nhit); 
        // change the mix value to tweak the effect
        float fresneleffect = mix(pow(1 - facingratio, 3), 1, 0.1); 
        // compute reflection direction (not need to normalize because all vectors
        // are already normalized)
        Vec3f refldir = raydir - nhit * 2 * raydir.dot(nhit); 
        refldir.normalize(); 
        Vec3f reflection = trace(phit + nhit * bias, refldir, spheres, depth + 1); 
        Vec3f refraction = 0; 
        // if the sphere is also transparent compute refraction ray (transmission)
        if (sphere->transparency) { 
            float ior = 1.1, eta = (inside) ? ior : 1 / ior; // are we inside or outside the surface? 
            float cosi = -nhit.dot(raydir); 
            float k = 1 - eta * eta * (1 - cosi * cosi); 
            Vec3f refrdir = raydir * eta + nhit * (eta *  cosi - sqrt(k)); 
            refrdir.normalize(); 
            refraction = trace(phit - nhit * bias, refrdir, spheres, depth + 1); 
        } 
        // the result is a mix of reflection and refraction (if the sphere is transparent)
        surfaceColor = ( 
            reflection * fresneleffect + 
            refraction * (1 - fresneleffect) * sphere->transparency) * sphere->surfaceColor; 
    } 
    else { 
        // it's a diffuse object, no need to raytrace any further
        for (unsigned i = 0; i < spheres.size(); ++i) { 
            if (spheres[i].emissionColor.x > 0) { 
                // this is a light
                Vec3f transmission = 1; 
                Vec3f lightDirection = spheres[i].center - phit; 
                lightDirection.normalize(); 
                for (unsigned j = 0; j < spheres.size(); ++j) { 
                    if (i != j) { 
                        float t0, t1; 
                        if (spheres[j].intersect(phit + nhit * bias, lightDirection, t0, t1)) { 
                            transmission = 0; 
                            break; 
                        } 
                    } 
                } 
                surfaceColor += sphere->surfaceColor * transmission * 
                std::max(float(0), nhit.dot(lightDirection)) * spheres[i].emissionColor; 
            } 
        } 
    }  
    return surfaceColor + sphere->emissionColor; 
}

Vec3f 
trace_box(
    const Vec3f &rayorig,
    const Vec3f &raydir,
    const std::vector<Box> &boxes,
    const int &depth)
{
    float tnear = INFINITY;
    const Box* box = NULL;
    // find intersection of this ray with the box in the scene
    for (unsigned i = 0; i < boxes.size(); ++i) {
        float t0 = INFINITY, t1 = INFINITY;
        if (boxes[i].intersect(rayorig, raydir, t0, t1)) {
            if (t0 < 0) t0 = t1;
            if (t0 < tnear) {
                tnear = t0;
                box = &boxes[i];
            }
            //std::cout << "here" << "\n";
        }
    }
    // if there's no intersection return black or background color
    if (!box) return Vec3f(2);
    //return box->surfaceColor;
    Vec3f surfaceColor = 0; // color of the ray/surfaceof the object intersected by the ray 
    Vec3f phit = rayorig + raydir * tnear; // point of intersection
    //std::cout << phit << "\n";
    //std::cout << box->center << "\n";
    Vec3f nhit = phit - box->center; // normal at the intersection point 
    nhit.normalize(); // normalize normal direction
    //std::cout << nhit << "\n";
    //std::cout << "Phit: " << phit << "Nhit: " << nhit << "\n";
    // If the normal and the view direction are not opposite to each other
    // reverse the normal direction. That also means we are inside the box so set
    // the inside bool to true. Finally reverse the sign of IdotN which we want
    // positive.
    float bias = 1e-4; // add some bias to the point from which we will be tracing 
    bool inside = false;
    if (raydir.dot(nhit) > 0) nhit = -nhit, inside = true;
    if ((box->transparency > 0 || box->reflection > 0) && depth < MAX_RAY_DEPTH) {
        float facingratio = -raydir.dot(nhit);
        // change the mix value to tweak the effect
        float fresneleffect = mix(pow(1 - facingratio, 3), 1, 0.1);
        // compute reflection direction (not need to normalize because all vectors
        // are already normalized)
        Vec3f refldir = raydir - nhit * 2 * raydir.dot(nhit);
        refldir.normalize();
        Vec3f reflection = trace_box(phit + nhit * bias, refldir, boxes, depth + 1);
        Vec3f refraction = 0;
        // if the box is also transparent compute refraction ray (transmission)
        if (box->transparency) {
            float ior = 1.1, eta = (inside) ? ior : 1 / ior; // are we inside or outside the surface? 
            float cosi = -nhit.dot(raydir);
            float k = 1 - eta * eta * (1 - cosi * cosi);
            Vec3f refrdir = raydir * eta + nhit * (eta *  cosi - sqrt(k));
            refrdir.normalize();
            refraction = trace_box(phit - nhit * bias, refrdir, boxes, depth + 1);
        }
        // the result is a mix of reflection and refraction (if the box is transparent)
        surfaceColor = (
            reflection * fresneleffect +
            refraction * (1 - fresneleffect) * box->transparency) * box->surfaceColor;
    }
    else {
        // it's a diffuse object, no need to raytrace any further
        for (unsigned i = 0; i < boxes.size(); ++i) {
            if (boxes[i].emissionColor.x > 0) {
                // this is a light
                Vec3f transmission = 1;
                Vec3f add = Vec3f(boxes[i].min + boxes[i].max);
                Vec3f center = Vec3f(add.x / 2.0, add.y / 2.0, add.z / 2.0);
                Vec3f lightDirection = center - phit;
                lightDirection.normalize();
                std::cout << "Nhit: " << nhit << "\n";
                std::cout << "Direction: " << lightDirection << "\n";
                for (unsigned j = 0; j < boxes.size(); ++j) {
                    if (i != j) {
                        float t0, t1;
                        if (boxes[j].intersect(phit + nhit * bias, lightDirection, t0, t1)) {
                            transmission = 0;
                            break;
                        }
                    }
                }
                if (nhit.dot(lightDirection) > 0) {
                    std::cout << nhit.dot(lightDirection) << "\n";
                }
                surfaceColor += box->surfaceColor * transmission *
                std::max(float(0), nhit.dot(lightDirection)) * boxes[i].emissionColor;
            }
        }
    }
    //std::cout << box->emissionColor << "\n";
    if (surfaceColor + box->emissionColor != Vec3f(0)) {
       //std::cout << surfaceColor + box->emissionColor << "\n"; 
    }
    return surfaceColor + box->emissionColor;
}

void render(const std::vector<Sphere> &spheres)
{
    unsigned width = 1200, height = 720; //640 480
    Vec3f *image = new Vec3f[width * height], *pixel = image;
    float invWidth = 1 / float(width), invHeight = 1 / float(height);
    float fov = 30, aspectratio = width / float(height);
    float angle = tan(M_PI * 0.5 * fov / 180.);
    // Trace rays
    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x, ++pixel) {
            float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
            float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
            Vec3f raydir(xx, yy, -1);
            raydir.normalize();
            *pixel = trace(Vec3f(0), raydir, spheres, 0);
        }
    }
    // Save result to a PPM image (keep these flags if you compile under Windows)
    std::ofstream ofs("./untitled.ppm", std::ios::out | std::ios::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (unsigned i = 0; i < width * height; ++i) {
        ofs << (unsigned char)(std::min(float(1), image[i].x) * 255) <<
               (unsigned char)(std::min(float(1), image[i].y) * 255) <<
               (unsigned char)(std::min(float(1), image[i].z) * 255);
    }
    ofs.close();
    delete [] image;
}

// Needs fix
void render_box(const std::vector<Box> &boxes) {
    unsigned width = 1200, height = 720; //640 480
    Vec3f *image = new Vec3f[width * height], *pixel = image;
    float invWidth = 1 / float(width), invHeight = 1 / float(height);
    float fov = 30, aspectratio = width / float(height);
    float angle = tan(M_PI * 0.5 * fov / 180.);
    // Trace rays
    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x, ++pixel) {
            float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
            float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
            Vec3f raydir(xx, yy, -1);
            raydir.normalize();
            //std::cout << "Raydir: " << raydir << "\n";
            *pixel = trace_box(Vec3f(0), raydir, boxes, 0);
        }
    }
    // Save result to a PPM image (keep these flags if you compile under Windows)
    std::ofstream ofs("./untitled.ppm", std::ios::out | std::ios::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (unsigned i = 0; i < width * height; ++i) {
        ofs << (unsigned char)(std::min(float(1), image[i].x) * 255) <<
               (unsigned char)(std::min(float(1), image[i].y) * 255) <<
               (unsigned char)(std::min(float(1), image[i].z) * 255);
    }
    ofs.close();
    delete [] image;

}

// Needs fix
void render_both(const std::vector<Sphere> &spheres, const std::vector<Box> &boxes) {
    unsigned width = 1200, height = 720; //640 480
    Vec3f *image = new Vec3f[width * height], *pixel = image;
    float invWidth = 1 / float(width), invHeight = 1 / float(height);
    float fov = 30, aspectratio = width / float(height);
    float angle = tan(M_PI * 0.5 * fov / 180.);
    // Trace rays
    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x, ++pixel) {
            float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
            float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
            Vec3f raydir(xx, yy, -1);
            raydir.normalize();
            *pixel = Vec3f(0) * trace_box(Vec3f(0), raydir, boxes, 0) + trace(Vec3f(0), raydir, spheres, 0);
        }
    }
    // Save result to a PPM image (keep these flags if you compile under Windows)
    std::ofstream ofs("./untitled.ppm", std::ios::out | std::ios::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (unsigned i = 0; i < width * height; ++i) {
        ofs << (unsigned char)(std::min(float(1), image[i].x) * 255) <<
               (unsigned char)(std::min(float(1), image[i].y) * 255) <<
               (unsigned char)(std::min(float(1), image[i].z) * 255);
    }
    ofs.close();
    delete [] image;


}

int main(int argc, char **argv)
{
    srand48(13);
    std::vector<Sphere> spheres;
    std::vector<Box> boxes;
    // position, radius, surface color, reflectivity, transparency, emission color
    spheres.push_back(Sphere(Vec3f( 1.0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0, 0.0));
    spheres.push_back(Sphere(Vec3f( 0.0,      0, -20),     4, Vec3f(1.00, 0.32, 0.36), 1, 0.5));
    spheres.push_back(Sphere(Vec3f( 5.0,     -1, -15),     2, Vec3f(0.90, 0.76, 0.46), 1, 0.0));
    spheres.push_back(Sphere(Vec3f( 5.0,      0, -25),     3, Vec3f(0.65, 0.77, 0.97), 1, 0.0));
    spheres.push_back(Sphere(Vec3f(-5.5,      0, -15),     3, Vec3f(0.90, 0.90, 0.90), 1, 0.0));
    // light
    spheres.push_back(Sphere(Vec3f( 0.0,     20, -30),     3, Vec3f(0.00, 0.00, 0.00), 0, 0.0, Vec3f(3)));
    //box light
    boxes.push_back(Box(Vec3f(0, 20, -10), Vec3f(20, 10, -5), Vec3f(0.20, 0.20, 0.20), 0, 0.0, Vec3f(3)));
    boxes.push_back(
        Box(Vec3f(-5, -5, -100), Vec3f(5, 5, -50), Vec3f(0.00, 255.00, 0.00), 1, 0.0));
    render_box(boxes);
    //render(spheres);

    return 0;
}
