#include <cstdlib> 
#include <cstdio> 
#include <cmath> 
#include <fstream> 
#include <vector> 
#include <iostream> 
#include <cassert>

// Helper class for vectors in R3
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

class Cube
{
public:
	Vec3f center;
	float sideLength, sideLength2;
	Vec3f surfaceColor, emissionColor;
	float transparency, reflection;
	Cube(
		const Vec3f &c,
		const float &sl,
		const Vec3f &sc,
		const float &refl = 0,
		const float & transp = 0,
		const Vec3f &ec = 0) :
		center(c), sideLength(sl), surfaceColor(sc), emissionColor(ec),
		transparency(transp), reflection(refl), sideLength2(sl * sl)
	{}

	bool intersect(const Vec3f &rayorig, const Vec3f &raydir, float &t0, float &t1) {
		Vec3f l = center - rayorig; 
		float tca = l.dot(raydir); 
		if (tca < 0) return false; 
		float d2 = l.dot(l) - tca * tca; 
		if (d2 > sideLength2) return false; 
		float thc = sqrt(sideLength2 - d2); 
		t0 = tca - thc; 
		t1 = tca + thc; 
	 
		return true; 
	}
