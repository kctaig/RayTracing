#pragma once

#include "head_include.hpp"
#include "camera.hpp"
#include "model.hpp"

class Scene
{
public:
	Scene() {}

	void render();
	PayLoad renderPixel(int x, int y);

	void addModel(Model* m) { models.push_back(m); }

	// attributes
	Camera cam;
	vector<Model*> models;
};