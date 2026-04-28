#pragma once

#include "Ray.hpp"
#include "light.hpp"
#include "payload.hpp"

class BBox;
class Material;

class Vertex {
  public:
    Vertex() = default;
    Vertex(vec3 p, vec3 n) : pos(p), normal(n), uv(vec2{0}) {}
    Vertex(vec3 p, vec3 n, vec2 uv) : pos(p), normal(n), uv(uv) {}

    vec3 pos;
    vec3 normal;
    vec2 uv;
};

class Mesh {
  public:
    bool intersection(const Ray& ray, PayLoad& payload) const;

    float calculateArea() const;
    vec2 getTexCoord(const vec2& uv) const;
    vec3 calculateNormal(const vec2& uv) const;

    vector<Vertex> vertices;
    shared_ptr<BBox> bboxPtr;
    shared_ptr<Material> material;
    shared_ptr<Light> light;
};

class Model {
  public:
    Model() = default;

    void loadFromFile(const string& fileDir, const string& fileName);

    // Getters
    auto& getLight() { return lights; }
    const auto& getLight() const { return lights; }
    auto& getMesh() { return meshes; }
    const auto& getMesh() const { return meshes; }
    auto& getMaterial() { return materials; }
    const auto& getMaterial() const { return materials; }

    void addLight(const shared_ptr<Light>& light) { lights.push_back(light); }

    shared_ptr<Light> randomSelectLight() const;

    float calculateLightsArea() const;

  private:
    vector<shared_ptr<Mesh>> meshes;
    vector<shared_ptr<Material>> materials;
    vector<shared_ptr<Light>> lights;
};