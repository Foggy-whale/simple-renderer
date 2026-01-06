#pragma once
#include <array>
#include "geometry.h"

struct Triangle {
    vec4 v[3]; // 三角形的三个顶点
    vec3 color[3], normal[3]; // 三角形三个顶点的颜色和法线
    vec2 tex_coord[3]; // 三角形三个顶点的纹理坐标
    vec3 world_pos[3]; // World position

    vec4 a() const { return v[0]; }
    vec4 b() const { return v[1]; }
    vec4 c() const { return v[2]; }

    Triangle& set_vertex(const int& ind, const vec4& ver); 
    Triangle& set_normal(const int& ind, const vec3& n); 
    Triangle& set_color(const int& ind, const vec3& c);
    Triangle& set_world_pos(const int& ind, const vec3& p);

    Triangle& set_normals(const std::array<vec3, 3>& normals);
    Triangle& set_colors(const std::array<vec3, 3>& colors);
    Triangle& set_tex_coord(const int& ind, const vec2& uv);
};