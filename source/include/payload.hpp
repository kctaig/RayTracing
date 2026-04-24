#pragma once

#include "bsdf.hpp"

class Mesh;

class PayLoad {
  public:
    PayLoad()
        : hitPos(vec3(0)),
          normal(vec3(0)),
          geomNormal(vec3(0)),
          uv(vec2(0)),
          t(FLT_MAX),
          frontFace(false) {}
    void initBxDFs();

    vec3 hitPos;
    vec3 normal;
    vec3 geomNormal;
    vec2 uv;
    float t;
    bool frontFace;

    shared_ptr<Mesh> mesh;
    shared_ptr<BSDF> bsdf;
};