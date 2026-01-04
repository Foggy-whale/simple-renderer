#include <cmath>
#include "camera.h"

constexpr double MY_PI = 3.1415926;

mat4 Camera::get_view_matrix() const {
    mat4 rotate, translate;

    vec3 lookat = (eye - target).normalized(), temp_up = up.normalized();
    if(fabs(temp_up * lookat) > 0.999f) {
        temp_up = (temp_up * lookat > 0) ? vec3(0, 0, -1) : vec3(0, 0, 1);
    }
    vec3 right  = cross_product(temp_up, lookat).normalized();
    vec3 new_up = cross_product(lookat, right).normalized();
    
    rotate << right.x, right.y, right.z, 0,
              new_up.x, new_up.y, new_up.z, 0,
              lookat.x, lookat.y, lookat.z, 0,
              0, 0, 0, 1;
    
    translate << 1, 0, 0, -eye.x,
                 0, 1, 0, -eye.y,
                 0, 0, 1, -eye.z,
                 0, 0, 0, 1;

    return rotate * translate;
}

mat4 Camera::get_projection_matrix() const {
    mat4 perspective;
    float half_fov = fov / 360. * MY_PI;
    perspective << 1 / (aspect * std::tan(half_fov)), 0, 0, 0,
                   0, 1 / std::tan(half_fov), 0, 0,
                   0, 0, (zNear + zFar) / (zNear - zFar), 2 * zNear * zFar / (zNear - zFar),
                   0, 0, -1, 0;

    return perspective;
}

Camera& Camera::set_eye(vec3 eye) {
    this->eye = eye;
    return *this;
}

Camera& Camera::set_target(vec3 target) {
    this->target = target;
    return *this;
}

Camera& Camera::set_up(vec3 up) {
    this->up = up;
    return *this;
}

Camera& Camera::set_projection(float fov, float aspect, float zNear, float zFar) {
    this->fov = fov;
    this->aspect = aspect;
    this->zNear = zNear;
    this->zFar = zFar;
    return *this;
}