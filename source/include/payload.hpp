#pragma once

#include "bsdf.hpp"

class Mesh;

class PayLoad {
  public:
    PayLoad() : pos(vec3(0)), normal(vec3(0)), uv(vec2(0)), t(FLT_MAX), frontFace(false) {}
    void initBxDFs();

    vec3 pos{0};
    vec3 normal{0};
    vec3 geoNormal{0};
    vec2 uv{0};
    float t{0};
    bool frontFace{false};

    shared_ptr<Mesh> mesh;
    shared_ptr<BSDF> bsdf;
};