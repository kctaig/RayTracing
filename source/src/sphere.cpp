#include "sphere.hpp"

bool Sphere::hit_sphere(const Ray& ray)
{
    vec3 oc = c - ray.getOrigin();
    auto a = dot(ray.getDir(), ray.getDir());
    auto b = -2.0 * dot(ray.getDir(), oc);
    auto c = dot(oc, oc) - r * r;
    auto t = b * b - 4 * a * c;
    return (t >= 0);
}
