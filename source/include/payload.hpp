#pragma once

#include "head_include.hpp"
 //#include "material.hpp"
#include "bsdf.hpp"

class Mesh;

class PayLoad
{
public:
    PayLoad() : hitPos(vec3(0)), normal(vec3(0)), uv(vec2(0)), t(FLT_MAX) {}
    void initBxDFs();
    
    float t;
    vec3 hitPos;
    vec3 normal;
    vec2 uv;
    shared_ptr<Mesh> meshPtr;
    shared_ptr<BSDF> bsdfPtr;
};