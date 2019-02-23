// Header file for geometry classes
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <fstream>

template<typename T>
class Vec3
{
public:
    T x, y, z;
    Vec3() : x(T(0)), y(T(0)), z(T(0)) {}
    Vec3(T xx) : x(xx), y(xx), z(xx) {}
    Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}
    Vec3& normalize()
    {
        T nor2 = length2();
        if (nor2 > 0) {
            T invNor = 1 / sqrt(nor2);
            x *= invNor, y *= invNor, z *= invNor;
        }
        return *this;
    }
    Vec3<T> operator * (const T &f) const { return Vec3<T>(x * f, y * f, z * f); } 
    Vec3<T> operator * (const Vec3<T> &v) const { return Vec3<T>(x * v.x, y * v.y, z * v.z); }
    T dot(const Vec3<T> &v) const { return x * v.x + y * v.y + z * v.z; } 
    Vec3<T> operator - (const Vec3<T> &v) const { return Vec3<T>(x - v.x, y - v.y, z - v.z); }
    Vec3<T> operator + (const Vec3<T> &v) const { return Vec3<T>(x + v.x, y + v.y, z + v.z); }
    Vec3<T>& operator += (const Vec3<T> &v) { x += v.x, y += v.y, z += v.z; return *this; }
    Vec3<T>& operator *= (const Vec3<T> &v) { x *= v.x, y *= v.y, z *= v.z; return *this; }
    bool operator != (const Vec3<T> &v) const { return x != v.x || y != v.y || z != v.z; }
    Vec3<T> operator - () const { return Vec3<T>(-x, -y, -z); }
    T length2() const { return x * x + y * y + z * z; }
    T length() const { return sqrt(length2()); }
    friend std::ostream & operator << (std::ostream &os, const Vec3<T> &v)
    {
        os << "[" << v.x << " " << v.y << " " << v.z << "]";
        return os;
    }
};

typedef Vec3<float> Vec3f;

class Shape
{
public: 
    Shape(
        const Vec3f &sc,
        const float &refl,
        const float &transp,
        const Vec3f &ec) :
        surfaceColor(sc), emissionColor(ec), transparency(transp),
        reflection(refl)
    {}
    virtual bool intersect(
        const Vec3f &rayorig, 
        const Vec3f &raydir, 
        float &t0, 
        float &t1) const = 0;
    Vec3f surfaceColor, emissionColor;      /// surface color and emission (light) 
    float transparency, reflection;         /// surface transparency and reflectivity
};

class Sphere : public Shape
{ 
public: 
    Vec3f center;                           /// position of the sphere 
    float radius, radius2;                  /// sphere radius and radius^2 
    Sphere(
        const Vec3f &c,
        const float &r,
        const Vec3f &sc,
        const float &refl = 0,
        const float &transp = 0,
        const Vec3f &ec = 0) :
        center(c), radius(r), radius2(r * r), Shape(sc, refl, transp, ec)
    {  }
    bool intersect(const Vec3f &rayorig, const Vec3f &raydir, float &t0, float &t1) const
    {
        Vec3f l = center - rayorig;
        float tca = l.dot(raydir);
        if (tca < 0) return false;
        float d2 = l.dot(l) - tca * tca;
        if (d2 > radius2) return false;
        float thc = sqrt(radius2 - d2);
        t0 = tca - thc;
        t1 = tca + thc;

        return true;
    }
};
class Box : public Shape
{
public:

    Vec3f min;
    Vec3f max;
    Vec3f add = min + max;
    Vec3f center = Vec3f(add.x / 2.0, add.y / 2.0, add.z / 2.0);
    Box(
        const Vec3f &min, 
        const Vec3f &max, 
        const Vec3f &sc, 
        const float &refl = 0,
        const float &transp = 0,
        const Vec3f &ec = 0) : min(min), max(max), Shape(sc, refl, transp, ec)
    {
    }
    bool intersect(const Vec3f &rayorig, const Vec3f &raydir, float &t0, float &t1) const
    {
        float tmin = (min.x - rayorig.x) / raydir.x;
        float tmax = (max.x - rayorig.x) / raydir.x;

        if (tmin > tmax) std::swap(tmin, tmax);

        float tymin = (min.y - rayorig.y) / raydir.y;
        float tymax = (max.y - rayorig.y) / raydir.y;

        if (tymin > tymax) std::swap(tymin, tymax);

        if ((tmin > tymax) || (tymin > tmax))
        return false;

        if (tymin > tmin)
        tmin = tymin;

        if (tymax < tmax)
        tmax = tymax;

        float tzmin = (min.z - rayorig.z) / raydir.z;
        float tzmax = (max.z - rayorig.z) / raydir.z;

        if (tzmin > tzmax) std::swap(tzmin, tzmax);

        if ((tmin > tzmax) || (tzmin > tmax))
        return false;

        if (tzmin > tmin)
        tmin = tzmin;

        if (tzmax < tmax)
        tmax = tzmax;

        if (false) {
            std::cout << "Max: " << std::to_string(tmax) << "\n";
            std::cout << "Min: " << std::to_string(tmin) << "\n";
        }

        t0 = tmin;
        t1 = tmax;

        return true;
    }
};

