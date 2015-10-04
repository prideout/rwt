/*
	Copyright 2006 Johannes Kopf (kopf@inf.uni-konstanz.de)

	Implementation of the algorithms described in the paper

	Recursive Wang Tiles for Real-Time Blue Noise
	Johannes Kopf, Daniel Cohen-Or, Oliver Deussen, Dani Lischinski
	In ACM Transactions on Graphics 25, 3 (Proc. SIGGRAPH 2006)

	If you use this software for research purposes, please cite
	the aforementioned paper in any resulting publication.
	
	You can find updated versions and other supplementary materials
	on our homepage:
	http://graphics.uni-konstanz.de/publications/2006/blue_noise
*/

// based on thatcher ulrichs geometry source --> thanks!

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <math.h>

class Vec2;
class Vec3;

/*******************
********************
****            ****
****    Vec2    ****
****            ****
********************
*******************/

class Vec2
{
public:
	Vec2(const Vec2 & v) { x = v.x; y = v.y; };
	Vec2(const Vec3 & v);
	Vec2() : x(0), y(0) {};
	Vec2(float _x, float _y) : x(_x), y(_y) {};

	Vec2 operator+(const Vec2& v) const { return Vec2(x+v.x, y+v.y); };
	Vec2& operator+=(const Vec2& v) { x += v.x; y += v.y; return *this; };

	Vec2 operator-(const Vec2& v) const { return Vec2(x-v.x, y-v.y); };
	Vec2& operator-=(const Vec2& v) { x -= v.x; y -= v.y; return *this; };
	
	Vec2 operator*(float f) const { return Vec2(x*f, y*f); };
	Vec2& operator*=(float f) { x *= f; y *= f; return *this; };
	
	Vec2 operator/(float f) const { return this->operator*(1.0f / f); }
	Vec2& operator/=(float f) { return this->operator*=(1.0f / f); }

	float operator*(const Vec2& v) const { return x*v.x + y*v.y; };

	float operator%(const Vec2& v) const { return x*v.y - y*v.x; };

	Vec2 operator-() const { return Vec2(-x, -y); };
	Vec2& normalize() { this->operator/=(sqrtf(x*x + y*y)); return *this; };

	float magnitude() { return sqrtf(x*x + y*y); };
	float sqrMagnitude() { return x*x + y*y; };

	Vec2 perp() { return Vec2(-y, x); };

	float get(int element) const { return (&x)[element]; }
	void set(int element, float NewValue) { (&x)[element] = NewValue; }

	float x, y;
};

/*******************
********************
****            ****
****    Vec3    ****
****            ****
********************
*******************/

class Vec3
{
public:
	Vec3(const Vec3& v) { x = v.x; y = v.y; z = v.z; };
	Vec3() : x(0), y(0), z(0) {};
	Vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {};
	Vec3(Vec2 & v) { x = v.x; y = v.y; z = 1.f; };

	bool operator!=(const Vec2 & v) const { return ((x != v.x) || (y != v.y)); };

	Vec3 operator+(const Vec3& v) const { return Vec3(x+v.x, y+v.y, z+v.z); };
	Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; };

	Vec3 operator-(const Vec3& v) const { return Vec3(x-v.x, y-v.y, z-v.z); };
	Vec3& operator-=(const Vec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; };
	
	Vec3 operator*(float f) const { return Vec3(x*f, y*f, z*f); };
	Vec3& operator*=(float f) { x *= f; y *= f; z *= f; return *this; };
	
	Vec3 operator/(float f) const { return this->operator*(1.0f / f); }
	Vec3& operator/=(float f) { return this->operator*=(1.0f / f); }

	float operator*(const Vec3& v) const { return x*v.x + y*v.y + z*v.z; };

	Vec3 operator%(const Vec3 & v) const { return Vec3(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); };

	Vec3 operator-() const { return Vec3(-x, -y, -z); };
	Vec3& normalize() { this->operator/=(sqrtf(x*x + y*y + z*z)); return *this; };

	float magnitude() { return sqrtf(x*x + y*y + z*z); };
	float sqrMagnitude() { return x*x + y*y + z*z; };

	float get(int element) const { return (&x)[element]; }
	void set(int element, float NewValue) { (&x)[element] = NewValue; }

	float x, y, z;
};

#endif