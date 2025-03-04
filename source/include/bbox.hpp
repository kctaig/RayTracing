#pragma once

#include "head_include.hpp"
#include "model.hpp"

class BBox {
public:
	BBox() {
		min = glm::vec3(FLT_MAX);
		max = glm::vec3(-FLT_MAX);
	}

	BBox(const vector<Vertex>& vertices);

	void unionMesh(const shared_ptr<Mesh> meshPtr);

	bool intersection(const Ray& ray) const;

	vec3 center();

	vec3 getMin() { return min; }	
	vec3 getMax() { return max; }

private:
	vec3 min;
	vec3 max;
};