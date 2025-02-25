#pragma once

#include "head_include.hpp"
#include "Ray.hpp"
#include "material.hpp"
#include "light.hpp"

class BBOX;

struct Vertex
{
	Vertex(vec3 pos) :pos(pos) {}
	vec3 pos;
	vec3 normal;
};

struct Mesh
{
	vector<int> indices;
	int matId;
	shared_ptr<BBOX> bboxPtr;
};

class Model
{
public:
	Model() {}
	Model(const std::string, const std::string, vector<Light>& lights);

	void modelInfo();

	vector<Material> mats;
	vector<Vertex> vertices;
	vector<std::shared_ptr<Mesh>> meshPtrs;
};