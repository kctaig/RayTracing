#pragma once

#include "head_include.hpp"

class Ray;
class Film;

class Camera {
  public:
    Camera() = default;

    Camera(const vec3 e, const vec3 l, const vec3 u, const float fovY)
        : eye(e), lookat(l), up(u), fovY(fovY) {};
    Ray rayCasting(const shared_ptr<Film>& filmPtr, const glm::ivec2& pixelCoord) const;

  private:
    vec3 eye{0};
    vec3 lookat{0};
    vec3 up{0};
    float fovY{0};
};