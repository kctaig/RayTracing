#pragma once

#include "head_include.hpp"
#include "Ray.hpp"
#include "material.hpp"
#include "light.hpp"

class BBox;

class Vertex
{
public:
	Vertex() = default;
	Vertex(vec3 p, vec3 n) :pos(p), normal(n), uv(vec2{ 0 }) {}
	Vertex(vec3 p, vec3 n, vec2 uv) :pos(p), normal(n), uv(uv) {}

	vec3 pos;
	vec3 normal;
	vec2 uv;
};

struct Mesh
{
	bool intersection(const Ray& ray, PayLoad& payload) const;
	float calculateArea();

	vector<Vertex> vertices;
	shared_ptr<BBox> bboxPtr;
	shared_ptr<Material> matPtr;
};

class Model
{
public:
	Model() {}

	void loadFromFile(const string, const string);

	vector<shared_ptr<Light>>& getLightPtrs() { return lightPtrs; }
	const vector<shared_ptr<Light>>& getLightPtrs() const { return lightPtrs; }

	vector<shared_ptr<Mesh>>& getMeshPtrs() { return meshPtrs; }
	const vector<shared_ptr<Mesh>>& getMeshPtrs() const { return meshPtrs; }

	void addLight(shared_ptr<Light> l) { lightPtrs.push_back(l); }

private:
	vector<shared_ptr<Mesh>> meshPtrs;
	vector<shared_ptr<Material>> matPtrs;
	vector<shared_ptr<Light>> lightPtrs;
};