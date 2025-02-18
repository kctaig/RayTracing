#pragma once

#include "head_include.hpp"
#include "Ray.hpp"

struct Vertex
{
	vec3 pos;
	vec3 normal;
};

struct Mesh
{
	vector<int> indices;
};

class Model
{
public:
	Model() {}
	Model(std::string fileNameDir, std::string fileName);

	void modelInfo();

	void intersection(const Ray&, PayLoad&, float&, float&) const;

private:
	vector<Vertex> vertices;
	vector<Mesh> triangles;
};