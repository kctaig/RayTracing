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
	vec3 renderPixel(int x, int y);

	void setModel(Model* m) { model = m; }
	void setNumSamples(int n) { numSamples = n; }
	PayLoad intersection(const Ray& ray) const;

	Ray genSecondaryRay(const PayLoad& payload) const;

	Camera cam;
	Model* model;
	int numSamples = 1;
	vector<Light> lights;
};