#pragma once

#include "head_include.hpp"
#include "camera.hpp"
#include "model.hpp"

class Scene
{
public:
	Scene() {}
	Scene(const std::string fileDir, const std::string fileName, bool test = false);
	void render();

	void setModel(Model* m) { model = m; }
	void setNumSamples(int n) { maxNumSample = n; }
	PayLoad intersection(const Ray& ray) const;
	vec3 rayCast(const Ray ray, int depth);
	vec3 sampleHemisphere(const vec3& normal);

	Camera cam;
	Model* model;
	int maxNumSample = 100;
	int maxDepth = 1;
	vector<Light> lights;
	float rr = 0.7;
};