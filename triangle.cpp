#include "triangle.h"

Triangle& Triangle::setVertex(int ind, vec4 ver) {
    assert(ind >= 0 && ind < 3);
    v[ind] = ver;
    return *this;
}

Triangle& Triangle::setNormal(int ind, vec3 n) {
    assert(ind >= 0 && ind < 3);
    normal[ind] = n;
    return *this;
}

Triangle& Triangle::setColor(int ind, float r, float g, float b) {
    assert(ind >= 0 && ind < 3);
    auto validate = [](float c) {
        return std::clamp(c / 255.0f, 0.0f, 1.0f);
    };

    color[ind] = vec3(validate(r), validate(g), validate(b));
    return *this;
}

Triangle &Triangle::setNormals(const std::array<vec3, 3>& normals) {
    for(int i = 0; i < 3; i++) normal[i] = normals[i];
    return *this;
}

Triangle& Triangle::setColors(const std::array<vec3, 3>& colors) {
    for(int i = 0; i < 3; i++) color[i] = colors[i];
    return *this;
}

Triangle &Triangle::setTexCoord(int ind, vec2 uv) {
    assert(ind >= 0 && ind < 3);
    tex_coord[ind] = uv;
    return *this;
}
