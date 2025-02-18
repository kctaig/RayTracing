#include "head_include.hpp"
#include "camera.hpp"
#include <scene.hpp>
#include <filesystem>

int main()
{
	Scene scene;

	// set camera
	Film film{ 50, 50 };
	// right coordinates
	Camera cam({ 278.0, 273.0, -800.0 }, { 278.0, 273.0, -799.0 }, { 0, 1.0, 0 }, 39.3077);
	cam.film = &film;
	scene.cam = cam;

	// set objects
	const std::string fileDir = std::filesystem::path("../../example-scenes-cg24/cornell-box").string();
	Model model(fileDir, "cornell-box.obj");
	//model.modelInfo();
	scene.addModel(&model);
	scene.render();

	return 0;
}