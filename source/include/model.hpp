#pragma once

#include "head_include.hpp"
#include "Ray.hpp"
#include "material.hpp"
#include "light.hpp"

class BBox;

struct Vertex
{
	Vertex(vec3 pos) :pos(pos) {}
	vec3 pos;
	vec3 normal;
};

class Model;

struct Mesh
{
	vector<int> indices;
	int matId;
	shared_ptr<BBox> bboxPtr;

	PayLoad intersection(const Ray& ray, const shared_ptr<Model>modelPtr) const;
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