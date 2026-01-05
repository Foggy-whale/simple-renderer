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

    Triangle& setVertex(int ind, vec4 ver); 
    Triangle& setNormal(int ind, vec3 n); 
    Triangle& setColor(int ind, float r, float g, float b);
    Triangle& setWorldPos(int ind, vec3 p);

    Triangle& setNormals(const std::array<vec3, 3>& normals);
    Triangle& setColors(const std::array<vec3, 3>& colors);
    Triangle& setTexCoord(int ind, vec2 uv);
};