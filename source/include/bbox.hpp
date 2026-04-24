#pragma once

#include "utils.hpp"

class Mesh;
class Vertex;
class Ray;

class BBox {
  public:
    BBox() : min(vec3(FLT_MAX)), max(vec3(-FLT_MAX)) {}

    explicit BBox(const vector<Vertex>& vertices);

    void unionMesh(const shared_ptr<Mesh>& mesh);

    bool intersection(const Ray& ray) const;

    int selectLongAxis() const;

    vec3 center() const;

    vec3 getMin() const { return min; }
    vec3 getMax() const { return max; }

  private:
    vec3 min;
    vec3 max;
};