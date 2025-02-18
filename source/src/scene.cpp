#include <cmath>
#include <filesystem>

#include "scene.hpp"
#include "log.hpp"

#define CURRENT_DIR std::filesystem::path(__FILE__).parent_path()

void Scene::render() {
	auto w = cam.film->width;
	auto h = cam.film->height;

	// Logger logger("D:/code/RayTracing/output/dragon.log");

#pragma omp parallel for
	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			PayLoad payload = renderPixel(i, j);
			vec3 color = { 0,0,0 };
			if (payload.ishit) color = glm::vec3{ payload.uv, 1 - payload.uv[0] - payload.uv[1] };
			cam.film->setPixel(i, j, color);
			if (i == 0) {
				cout << static_cast<float>(j) / static_cast<float>(h) << endl;
			}
		}
	}
	// logger.RestoreOriginalBuffers();
	cam.film->saveToFile(CURRENT_DIR / "../../output/image.ppm");
}

PayLoad Scene::renderPixel(int x, int y) {
	Ray ray = cam.generateRay({ x, y }, { 0.5, 0.5 });
	float tMin = 0.f;
	float tMax = INFINITY;
	PayLoad payload{};
	for (size_t i = 0; i < models.size(); i++) {
		models[i]->intersection(ray, payload, tMin, tMax);
	}
	return payload;
}