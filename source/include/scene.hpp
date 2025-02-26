#pragma once

#include "head_include.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "bvh.hpp"

class Scene
{
public:
	Scene() {}
	Scene(const std::string fileDir, const std::string fileName, bool test = false);
	void render();

	void setModel(shared_ptr<Model>m) { modelPtr = m; }
	void BVHBuild() { bvhPtr = std::make_shared<BVH>(*modelPtr, modelPtr->meshPtrs); }
	void setNumSamples(int n) { maxNumSample = n; }
	void setMaxDepth(int n) { maxDepth = n; }
	bool intersection(const Ray& ray, PayLoad& payload) const;
	bool intersection(const Ray& ray, const shared_ptr<Model> modelPtr, PayLoad& payload) const;
	vec3 rayTracing(const Ray& ray, int depth);
	vec3 sampleHemisphere(const vec3& normal);

	Camera cam;
	shared_ptr<Model> modelPtr;
	shared_ptr<BVH>bvhPtr;
	int maxNumSample = 100;
	int maxDepth = 1;
	vector<Light> lights;
	float rr = 0.7;
};