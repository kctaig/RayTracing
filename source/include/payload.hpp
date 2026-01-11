#pragma once

#include "bsdf.hpp"
#include "head_include.hpp"

class Mesh;

class PayLoad {
  public:
    PayLoad() : hitPos(vec3(0)), normal(vec3(0)), uv(vec2(0)), t(FLT_MAX) {}
    void initBxDFs();

    vec3 hitPos;
    vec3 normal;
    vec2 uv;
    float t;

    shared_ptr<Mesh> mesh;
    shared_ptr<BSDF> bsdf;
};