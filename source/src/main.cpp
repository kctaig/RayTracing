#include "head_include.hpp"
#include "camera.hpp"
#include "scene.hpp"

int main()
{
	const std::string sceneDir = "../../example-scenes-cg24";
	//const std::string fileName = "cornell-box";
	const std::string fileName = "veach-mis";
	//const std::string fileName = "cornell-box";
	Scene scene(sceneDir + "/" + fileName, fileName);
	scene.cam.filmPtr->scale(0.5f);
	shared_ptr<Model> modelPtr = std::make_shared<Model>(sceneDir + "/" + fileName, fileName, scene.lights);
	scene.setModel(modelPtr);
	scene.BVHBuild();
	scene.setMaxDepth(10);
	scene.setNumSamples(100000);
	scene.render();
	return 0;
}