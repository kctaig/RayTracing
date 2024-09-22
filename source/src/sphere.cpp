#include "sphere.hpp"

bool Sphere::hit_sphere(const Ray& ray)
{
    vec3 oc = c - ray.get_origin();
    auto a = dot(ray.get_dir(), ray.get_dir());
    auto b = -2.0 * dot(ray.get_dir(), oc);
    auto c = dot(oc, oc) - r * r;
    auto t = b * b - 4 * a * c;
    return (t >= 0);
}
