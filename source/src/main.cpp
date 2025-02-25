#include "head_include.hpp"
#include "camera.hpp"
#include "scene.hpp"

int main()
{
	const std::string sceneDir = "../../example-scenes-cg24";
	const std::string fileName = "cornell-box";
	Scene scene(sceneDir + "/" + fileName, fileName, true);
	shared_ptr<Model> modelPtr = std::make_shared<Model>(sceneDir + "/" + fileName, fileName, scene.lights);
	scene.setModel(modelPtr);
	scene.BVHBuild();
	scene.setMaxDepth(1);
	scene.setNumSamples(100);
	scene.render();
	return 0;
}