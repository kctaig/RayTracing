#pragma once
#include "head_include.hpp"
#include <ray.hpp>

class Sphere {
public:

    Sphere(vec3 c, float r) :c(c), r(r) {};
    bool hit_sphere(const Ray& ray);

private:
    vec3 c;
    float r;
};