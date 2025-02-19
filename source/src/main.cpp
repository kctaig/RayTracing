#include "head_include.hpp"
#include "camera.hpp"
#include "scene.hpp"
#include <chrono>

int main()
{
	const std::string sceneDir = "../../example-scenes-cg24";
	const std::string fileName = "cornell-box";
	Scene scene(sceneDir + "/" + fileName, fileName, true);
	Model model(sceneDir + "/" + fileName, fileName);
	scene.setModel(&model);
	auto start = std::chrono::high_resolution_clock::now();
	scene.render();
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
	std::cout << "Elapsed time: " << duration.count() << " seconds" << std::endl;

	return 0;
}