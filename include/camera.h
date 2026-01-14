#pragma once
#include "geometry.h"
#include "global.h"

class Camera {
private:
    vec3 eye = {0, 0, 1}, target = {0, 0, 0}, up = {0, 1, 0};
    float fov = 45.0f, aspect = (float)width / height, zNear = 0.1f, zFar = 100.f;
public:
    Camera() = default;

    Camera& set_eye(vec3 eye);
    Camera& set_target(vec3 target);
    Camera& set_up(vec3 up);
    Camera& set_projection(float fov, float aspect, float zNear, float zFar);

    vec3 get_eye() const { return eye; }
    vec3 get_target() const { return target; }
    vec3 get_up() const { return up; }
    
    mat4 get_view_matrix() const;
    mat4 get_projection_matrix() const;
};