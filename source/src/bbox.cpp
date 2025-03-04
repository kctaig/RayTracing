#include "bbox.hpp"

BBox::BBox(const vector<Vertex>& vertices)
{
	min = glm::vec3(FLT_MAX);
	max = glm::vec3(-FLT_MAX);	
	for (const Vertex& v : vertices)
	{
		min = glm::min(min, v.pos);
		max = glm::max(max, v.pos);
	}
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

vec3 BBox::center()
{
	return vec3{ 0.5f * (min + max) };
}