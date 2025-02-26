#include "bbox.hpp"

BBox::BBox(const vector<Vertex>& vertices, const vector<int>& indices) :BBox()
{
	for (auto idx : indices)
	{
		vec3 pos = vertices[idx].pos;
		min = glm::min(min, pos);
		max = glm::max(max, pos);
	}
}

BBox::BBox(const BBox& b1, const BBox& b2)
{
	min = glm::min(b1.min, b2.min);
	max = glm::max(b1.max, b2.max);
}

void BBox::unionMesh(const shared_ptr<Mesh> meshPtr)
{
	min = glm::min(min, meshPtr->bboxPtr->min);
	max = glm::max(max, meshPtr->bboxPtr->max);
}

bool BBox::intersection(const Ray& ray) const
{
	vec3 invDir = 1.0f / ray.getDir();
	vec3 tmin = (min - ray.getOrigin()) * invDir;
	vec3 tmax = (max - ray.getOrigin()) * invDir;

	vec3 t1 = glm::min(tmin, tmax);
	vec3 t2 = glm::max(tmin, tmax);
	float tNear = glm::max(t1.x, glm::max(t1.y, t1.z));
	float tFar = glm::min(t2.x, glm::min(t2.y, t2.z));

	return tNear <= tFar && tFar >= 0.0f;
}