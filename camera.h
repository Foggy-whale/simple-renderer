#pragma once
#include "geometry.h"

class Camera {
private:
    vec3 eye, target, up;
    float fov, aspect, zNear, zFar;
public:
    Camera() = default;
    Camera(vec3 eye, float fov, float aspect, float zNear, float zFar)
        : eye(eye), target(eye + vec3(0, 0, 1)), up(vec3(0, 1, 0)), fov(fov), aspect(aspect), zNear(zNear), zFar(zFar) {}
    Camera(vec3 eye, vec3 target, float fov, float aspect, float zNear, float zFar)
        : eye(eye), target(target), up(vec3(0, 1, 0)), fov(fov), aspect(aspect), zNear(zNear), zFar(zFar) {}
    Camera(vec3 eye, vec3 target, vec3 up, float fov, float aspect, float zNear, float zFar)
        : eye(eye), target(target), up(up), fov(fov), aspect(aspect), zNear(zNear), zFar(zFar) {}

    Camera& set_eye(vec3 eye);
    Camera& set_target(vec3 target);
    Camera& set_up(vec3 up);
    Camera& set_projection(float fov, float aspect, float zNear, float zFar);

    mat4 get_view_matrix() const;
    mat4 get_projection_matrix() const;
    
    vec3 get_eye() const { return eye; }
};