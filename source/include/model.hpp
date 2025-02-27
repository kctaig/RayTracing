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

	bool intersection(const Ray& ray, PayLoad& payload, const shared_ptr<Model>modelPtr) const;
	vec3 sampleMesh(const shared_ptr<Model>modelPtr);
};

class Model
{
public:
	Model() {}
	Model(const std::string, const std::string, vector<Light>& lights);

	void modelInfo();

	vector<Material> modelMats;
	vector<Vertex> vertices;
	vector<std::shared_ptr<Mesh>> meshPtrs;
};