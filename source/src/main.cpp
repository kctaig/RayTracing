#include "head_include.hpp"
#include "camera.hpp"
#include "scene.hpp"

int main()
{
	const std::string sceneDir = "../../example-scenes-cg24";
	const std::string fileName = "cornell-box";
	Scene scene(sceneDir + "/" + fileName, fileName, true);
	Model model(sceneDir + "/" + fileName, fileName, scene.lights);
	scene.setModel(&model);
	scene.setNumSamples(100);
	scene.render();
	return 0;
}