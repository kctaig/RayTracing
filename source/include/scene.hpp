#pragma once

#include "head_include.hpp"
#include <camera.hpp>
#include <tri_mesh_model.hpp>
#include <sphere.hpp>

class Scene
{
public:
	Scene(){}
	Scene(const Camera& cam, const vector<Model*> m):cam(cam),models(m){}

	void renderScene();
	PayLoad renderPixel(size_t x, size_t y);

	void add_sphere(Sphere s) { spheres.push_back(s); }
	void addModel(Model *m) { models.push_back(m); }

	// attributes
	Camera cam;
	vector<Model*> models;
	vector<Sphere> spheres;
};