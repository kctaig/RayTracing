#pragma once

#include "head_include.hpp"
#include <camera.hpp>
#include <tri_model.hpp>
#include <ray.hpp>
#include <sphere.hpp>
#include <frame_buffer.hpp>

class Scene
{
public:
	Scene() {}

	void render();
	glm::vec3 render_per_pixel(vec3 p, size_t x, size_t y);

	// set
	void add_sphere(Sphere s) { spheres.push_back(s); }
	void add_tri_model(Tri_Model t) { tri_models.push_back(t); }
	void set_cam(Camera c) { cam = c; }
	void set_width(size_t w) { image_width = w; }
	void set_height(size_t h) { image_height = h; }

	// attributes
	size_t image_width, image_height;
	Camera cam;
	vector<Tri_Model> tri_models;
	vector<Sphere> spheres;
};