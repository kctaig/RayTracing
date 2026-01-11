#pragma once

#include "head_include.hpp"

class Ray {
  public:
    Ray(const vec3 pos, const vec3 dir) : origin(pos), dir(dir) {}

    vec3 getOrigin() const { return origin; }
    vec3 getDir() const { return dir; }
    vec3 at(const float t) const { return origin + dir * t; }

  private:
    vec3 origin;
    vec3 dir;
};