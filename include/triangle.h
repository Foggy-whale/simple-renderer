#pragma once
#include <array>
#include "geometry.h"

struct Triangle {
    vec4 v[3]; // 三角形的三个顶点
    vec3 world_pos[3]; // 三角形三个顶点的世界坐标
    vec3 color[3];  // 三角形三个顶点的颜色
    vec3 normal[3]; // 三角形三个顶点的法线
    vec2 tex_coord[3]; // 三角形三个顶点的纹理坐标
    vec3 tangent[3]; // 三角形三个顶点的切线
    vec3 bitangent[3]; // 三角形三个顶点的副切线

    vec4 a() const { return v[0]; }
    vec4 b() const { return v[1]; }
    vec4 c() const { return v[2]; }

    Triangle& set_vertex(const int& ind, const vec4& ver); 
    Triangle& set_normal(const int& ind, const vec3& n); 
    Triangle& set_color(const int& ind, const vec3& c);
    Triangle& set_world_pos(const int& ind, const vec3& p);
    Triangle& set_tex_coord(const int& ind, const vec2& uv);
    Triangle& set_tangent(const int& ind, const vec3& t);
    Triangle& set_bitangent(const int& ind, const vec3& b);
};