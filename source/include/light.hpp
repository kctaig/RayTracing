#pragma once

#include "utils.hpp"

class Mesh;

class Light {
  public:
    Light() = default;

    string getMatName() const { return matName; }
    vec3 getRadiance() const { return radiance; }
    float getArea() const { return area; }
    auto& getMeshes() const { return meshes; }
    auto& getMeshes() { return meshes; }

    void setMatName(const string& name) { matName = name; }
    void setRadiance(const vec3& rad) { radiance = rad; }
    void addArea(const float a) { area += a; }

  private:
    string matName = "nan-mat";
    vec3 radiance{0};
    float area = 0.f;
    vector<shared_ptr<Mesh>> meshes;
};