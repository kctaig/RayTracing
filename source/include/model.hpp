#pragma once

#include "head_include.hpp"
#include "Ray.hpp"
#include "material.hpp"

struct Vertex
{
	vec3 pos;
	vec3 normal;
};

struct Mesh
{
	vector<int> indices;
	int mat_id;
};

class Model
{
public:
	Model() {}
	Model(const std::string, const std::string);

	void modelInfo();

	void intersection(const Ray &, PayLoad &, float &, float &) const;

	vector<Material> mats;

	vector<Vertex> vertices;
	vector<Mesh> triangles;
};