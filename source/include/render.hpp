#pragma once

#include "head_include.hpp"

class Camera;
class Film;
class Model;
class BVH;
class Ray;
class PayLoad;
class Sampler;

class Render {
  public:
    Render() = default;
    ~Render() = default;

    Render(const string& sceneDir, const string& fileName);

    // loop intersection
    bool iterIntersection(const Ray& ray, PayLoad& payload) const;
    // bvh intersection
    bool intersection(const Ray& ray, PayLoad& payload) const;
    vec3 rayTracing(const Ray& ray, PayLoad& currentPayload, int depth);

    vec3 rayTest(const Ray& ray) const;

    void setNumSamples(const int n) { maxNumSample = n; }
    void setMaxDepth(const int n) { maxDepth = n; }
    void setModel(const shared_ptr<Model>& m) { model = m; }
    void setNumIter(const int num) { numIter = num; }
    void setDebugNormalMode(bool enabled) { debugNormalMode = enabled; }
    void BVHBuild();

    shared_ptr<Sampler> sampleLight(const PayLoad& payload) const;

    void render();
    void renderOneSample(int sampleIndex);

    shared_ptr<Camera> camera;
    shared_ptr<Film> film;
    shared_ptr<Model> model;
    shared_ptr<BVH> bvh;

  private:
    string fileName;
    int maxNumSample = 100;
    int maxDepth = 5;
    int numIter = 50;
    float rrThreshold = 0.7f;
    bool debugNormalMode = false;
};
