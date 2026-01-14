#include "triangle.h"

Triangle& Triangle::set_vertex(const int& ind, const vec4& ver) {
    assert(ind >= 0 && ind < 3);
    v[ind] = ver;
    return *this;
}

Triangle& Triangle::set_normal(const int& ind, const vec3& n) {
    assert(ind >= 0 && ind < 3);
    normal[ind] = n;
    return *this;
}

Triangle& Triangle::set_color(const int& ind, const vec3& c) {
    assert(ind >= 0 && ind < 3);
    color[ind] = c;
    return *this;
}

Triangle &Triangle::set_world_pos(const int& ind, const vec3& p) {
    assert(ind >= 0 && ind < 3);
    world_pos[ind] = p;
    return *this;
}

Triangle &Triangle::set_tex_coord(const int& ind, const vec2& uv) {
    assert(ind >= 0 && ind < 3);
    tex_coord[ind] = uv;
    return *this;
}

Triangle &Triangle::set_tangent(const int& ind, const vec3& t) {
    assert(ind >= 0 && ind < 3);
    tangent[ind] = t;
    return *this;
}

Triangle &Triangle::set_bitangent(const int& ind, const vec3& b) {
    assert(ind >= 0 && ind < 3);
    bitangent[ind] = b;
    return *this;
}
