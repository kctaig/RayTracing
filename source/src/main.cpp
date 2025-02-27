#include "head_include.hpp"
#include "camera.hpp"
#include "scene.hpp"

int main()
{
	const std::string sceneDir = "../../example-scenes-cg24";
	const std::string fileName = "cornell-box";
	//const std::string fileName = "veach-mis";
	Scene scene(sceneDir + "/" + fileName, fileName);
	scene.cam.filmPtr->reset(200, 200);
	shared_ptr<Model> modelPtr = std::make_shared<Model>(sceneDir + "/" + fileName, fileName, scene.lights);
	scene.setModel(modelPtr);
	scene.BVHBuild();
	scene.setMaxDepth(5);
	scene.setNumSamples(10000);
	scene.render();
	return 0;
}