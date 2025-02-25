#pragma once

#include "head_include.hpp"
#include "model.hpp"

class BBOX {
public:
	BBOX() {
		min = glm::vec3(FLT_MAX);
		max = glm::vec3(-FLT_MAX);
	}

	BBOX(glm::vec3 min, glm::vec3 max)
		: min(min), max(max) {
	}

	BBOX(const vector<Vertex>& vertices, const vector<int>& indices);

	void unionMesh(const shared_ptr<Mesh> meshPtr);

	vec3 min;
	vec3 max;
};