#pragma once

#include "head_include.hpp"
#include "camera.hpp"
#include "model.hpp"
#include "bvh.hpp"
#include "sampler.hpp"

class Scene
{
public:
	Scene() = default;
	~Scene() = default;

	Scene(const string fileDir, string fileName);

	// loop intersection
	bool iterIntersection(const Ray& ray, PayLoad& payload) const;
	// bvh intersection
	bool intersection(const Ray& ray, PayLoad& payload) const;
	vec3 rayTracing(const Ray& ray, PayLoad& currentPayload, int depth);

	vec3 rayTest(const Ray& ray) const;

	shared_ptr<Camera> getCamera() { return camPtr; }
	shared_ptr<Film> getFilm() { return filmPtr; }
	shared_ptr<Model> getModel() { return modelPtr; }
	shared_ptr<BVH> getBVH() { return bvhPtr; }

	void setNumSamples(int n) { maxNumSample = n; }
	void setMaxDepth(int n) { maxDepth = n; }
	void setModel(shared_ptr<Model>m) { modelPtr = m; }
	void setNumIter(int num) { numIter = num; }
	void BVHBuild();

	shared_ptr<Sampler> sampleLight(const PayLoad& payload) const;
	void render();

private:
	string fileName;
	shared_ptr<Camera>camPtr;
	shared_ptr<Film>filmPtr;
	shared_ptr<Model> modelPtr;
	shared_ptr<BVH>bvhPtr;
	int maxNumSample = 100;
	int maxDepth = 5;
	int numIter = 50;
	float rrThreshold = 0.7f;
};