#pragma once

#include <math.h>

struct Vector2Df
{
	union {
		struct { float x, y; };
		float _v[2];
	};

	Vector2Df(float _x = 0, float _y = 0) : x(_x), y(_y) {}
	Vector2Df(const Vector2Df& v) : x(v.x), y(v.y) {}
};

struct Vector3Df
{
	union {
		struct { float x, y, z, w; };
		float _v[4];
	};

	Vector3Df(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
	Vector3Df(const Vector3Df& v) : x(v.x), y(v.y), z(v.z) {}
	inline float lengthsq(){ return x*x + y*y + z*z; }
	inline void normalize(){ float norm = sqrtf(x*x + y*y + z*z); x /= norm; y /= norm; z /= norm; }
	inline Vector3Df& operator+=(const Vector3Df& v){ x += v.x; y += v.y; z += v.z; return *this; }
	inline Vector3Df& operator-=(const Vector3Df& v){ x -= v.x; y -= v.y; z -= v.z; return *this; }
	inline Vector3Df& operator*=(const float& a){ x *= a; y *= a; z *= a; return *this; }
	inline Vector3Df& operator*=(const Vector3Df& v){ x *= v.x; y *= v.y; z *= v.z; return *this; }
	inline Vector3Df operator*(float a) const{ return Vector3Df(x*a, y*a, z*a); }
	inline Vector3Df operator/(float a) const{ return Vector3Df(x / a, y / a, z / a); }
	inline Vector3Df operator*(const Vector3Df& v) const{ return Vector3Df(x * v.x, y * v.y, z * v.z); }
	inline Vector3Df operator+(const Vector3Df& v) const{ return Vector3Df(x + v.x, y + v.y, z + v.z); }
	inline Vector3Df operator-(const Vector3Df& v) const{ return Vector3Df(x - v.x, y - v.y, z - v.z); }
	inline Vector3Df& operator/=(const float& a){ x /= a; y /= a; z /= a; return *this; }
	inline bool operator!=(const Vector3Df& v){ return x != v.x || y != v.y || z != v.z; }
};




inline Vector3Df min3(const Vector3Df& v1, const Vector3Df& v2){ return Vector3Df(v1.x < v2.x ? v1.x : v2.x, v1.y < v2.y ? v1.y : v2.y, v1.z < v2.z ? v1.z : v2.z); }
inline Vector3Df max3(const Vector3Df& v1, const Vector3Df& v2){ return Vector3Df(v1.x > v2.x ? v1.x : v2.x, v1.y > v2.y ? v1.y : v2.y, v1.z > v2.z ? v1.z : v2.z); }
inline Vector3Df cross(const Vector3Df& v1, const Vector3Df& v2){ return Vector3Df(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x); }
inline float dot(const Vector3Df& v1, const Vector3Df& v2){ return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z; }
// inline float dot(const Vector3Df& v1, const float4& v2){ return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z; }
// inline float dot(const float4& v1, const Vector3Df& v2){ return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z; }
inline float distancesq(const Vector3Df& v1, const Vector3Df& v2){ return (v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y) + (v1.z - v2.z)*(v1.z - v2.z); }
inline float distance(const Vector3Df& v1, const Vector3Df& v2){ return sqrtf((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y) + (v1.z - v2.z)*(v1.z - v2.z)); }
