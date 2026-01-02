#pragma once
#include <cmath>
#include <cassert>
#include <iostream>

template<int n> struct vec {
    float data[n] = {0};
    float& operator[](const int i)       { assert(i>=0 && i<n); return data[i]; }
    float  operator[](const int i) const { assert(i>=0 && i<n); return data[i]; }
};

template<int n> std::ostream& operator<<(std::ostream& out, const vec<n>& v) {
    for (int i=0; i<n; i++) out << v[i] << " ";
    return out;
}

template<> struct vec<2> {
    float x = 0, y = 0;
    vec<2>() = default;
    vec<2> (float x, float y) : x(x), y(y) {}

    float& operator[](const int i)       { assert(i>=0 && i<2); return i ? y : x; }
    float  operator[](const int i) const { assert(i>=0 && i<2); return i ? y : x; }
    vec<2> operator+(const vec<2>& v) const { return vec<2>(x+v.x, y+v.y); }
    vec<2> operator-(const vec<2>& v) const { return vec<2>(x-v.x, y-v.y); }
    vec<2> operator*(const float s) const { return vec<2>(x*s, y*s); }
    vec<2> operator/(const float s) const { return vec<2>(x/s, y/s); }
};

template<> struct vec<3> {
    float x = 0, y = 0, z = 0;
    vec<3>() = default;
    vec<3> (float x, float y, float z) : x(x), y(y), z(z) {}

    float& operator[](const int i)       { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
    float  operator[](const int i) const { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
    vec<3> operator+(const vec<3>& v) const { return vec<3>(x+v.x, y+v.y, z+v.z); }
    vec<3> operator-(const vec<3>& v) const { return vec<3>(x-v.x, y-v.y, z-v.z); }
    vec<3> operator*(const float s) const { return vec<3>(x*s, y*s, z*s); }
    vec<3> operator/(const float s) const { return vec<3>(x/s, y/s, z/s); }
};

typedef vec<2> vec2;
typedef vec<3> vec3;
