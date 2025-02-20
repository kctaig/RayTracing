#pragma once

#include "head_include.hpp"
#include "Ray.hpp"
#include "material.hpp"
#include "light.hpp"

struct Vertex
{
	Vertex(vec3 pos):pos(pos) {}
	vec3 pos;
	vec3 normal;
};

struct Mesh
{
	vector<int> indices;
	int matId;
};

class Model
{
public:
	Model() {}
	Model(const std::string, const std::string, vector<Light>&lights);

	void modelInfo();

	vector<Material> mats;
	vector<Vertex> vertices;
	vector<Mesh> triangles;
};