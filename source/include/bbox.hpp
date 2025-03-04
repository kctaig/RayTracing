#pragma once

#include "head_include.hpp"
#include "model.hpp"

class BBox {
public:
	BBox() {
		min = glm::vec3(FLT_MAX);
		max = glm::vec3(-FLT_MAX);
	}

	BBox(glm::vec3 min, glm::vec3 max)
		: min(min), max(max) {
	}

	BBox(const vector<Vertex>& vertices);
	BBox(const BBox& b1, const BBox& b2);

	void unionMesh(const shared_ptr<Mesh> meshPtr);

	bool intersection(const Ray& ray) const;

	vec3 center();

	vec3 min;
	vec3 max;
};