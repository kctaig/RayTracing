#include "bbox.hpp"

BBOX::BBOX(const vector<Vertex>& vertices, const vector<int>& indices)
{
	for (auto idx : indices)
	{
		vec3 pos = vertices[idx].pos;
		min = glm::min(min, pos);
		max = glm::max(max, pos);
	}
}

void BBOX::unionMesh(const shared_ptr<Mesh> meshPtr)
{
	min = glm::min(min, meshPtr->bboxPtr->min);
	max = glm::max(max, meshPtr->bboxPtr->max);
}