#pragma once

#include "head_include.hpp"
#include "camera.hpp"
#include "sphere.hpp"
#include "model.hpp"

class Scene
{
public:
	Scene() {}

	void render();
	PayLoad renderPixel(size_t x, size_t y);

	void add_sphere(Sphere s) { spheres.push_back(s); }
	void addModel(Model* m) { models.push_back(m); }

	// attributes
	Camera cam;
	vector<Model*> models;
	vector<Sphere> spheres;
};