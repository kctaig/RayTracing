#pragma once

#include "head_include.hpp"

struct Mesh;

class Light
{
public:
	Light() = default;

	string getMatName() const { return matName; }
	vec3 getRadiance() const { return radiance; }
	float getArea() const { return area; }
	const vector<shared_ptr<Mesh>>& getMeshPtrs() const { return meshPtrs; }
	vector<shared_ptr<Mesh>>& getMeshPtrs() { return meshPtrs; }

	void setMatName(const string& name) { matName = name; }
	void setRadiance(const vec3& rad) { radiance = rad; }
	void addArea(float a) { area += a; }

private:

	string matName = "nan-mat";
	vec3 radiance{ 0 };
	float area = 0.f;
	vector<shared_ptr<Mesh>>meshPtrs;
};