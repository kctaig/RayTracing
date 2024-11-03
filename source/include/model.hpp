#pragma once

#include "head_include.hpp"
#include "Ray.hpp"

class Vertex
{

public:
    vec3 pos;
    vec3 normal;
    vec3 color;
};

class Mesh
{

public:
    vector<size_t> indices;
};

class Model
{

public:
    Model() {}
    virtual void intersection(const Ray&, PayLoad&, float&, float&) const = 0;

    vector<Vertex> vertices;
};
