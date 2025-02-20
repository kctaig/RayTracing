#pragma once

#include "head_include.hpp"

struct Mesh;

class Light
{
public:
    Light() {}

    vector<Mesh>meshes;
    string matName;
    vec3 radiance;
};
