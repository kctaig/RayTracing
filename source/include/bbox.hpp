#pragma once

#include "head_include.hpp"
#include "model.hpp"

class BBox {
public:
	BBox():min(vec3(FLT_MAX)),max(vec3(-FLT_MAX)) {}

	BBox(const vector<Vertex>& vertices);

	void unionMesh(const shared_ptr<Mesh> meshPtr);

	bool intersection(const Ray& ray) const;

	vec3 center() const;

	vec3 getMin() const { return min; }
	vec3 getMax() const { return max; }

private:
	vec3 min;
	vec3 max;
};