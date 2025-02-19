#include <cmath>
#include <filesystem>

#include "scene.hpp"
#include "log.hpp"
#include "tinyxml2/tinyxml2.h"

Scene::Scene(const std::string sceneDir, const std::string fileName, bool test) {
	using namespace tinyxml2;

	// 加载 XML 文件
	const std::string xmlFile = sceneDir + "/" + fileName + ".xml";

	XMLDocument doc;
	if (doc.LoadFile(xmlFile.c_str()) != XML_SUCCESS) {
		std::cerr << "can't load XML file: " << xmlFile << endl;
		return;
	}

	// 读取摄像机数据
	XMLElement* cameraElement = doc.FirstChildElement("camera");
	if (cameraElement) {
		int width = cameraElement->IntAttribute("width");
		int height = cameraElement->IntAttribute("height");
		float fovy = cameraElement->FloatAttribute("fovy");

		// 读取 <eye> 元素
		XMLElement* eyeElement = cameraElement->FirstChildElement("eye");
		vec3 eye = vec3(
			eyeElement->FloatAttribute("x"),
			eyeElement->FloatAttribute("y"),
			eyeElement->FloatAttribute("z")
		);

		// 读取 <lookat> 元素
		XMLElement* lookatElement = cameraElement->FirstChildElement("lookat");
		vec3 lookat = vec3(
			lookatElement->FloatAttribute("x"),
			lookatElement->FloatAttribute("y"),
			lookatElement->FloatAttribute("z")
		);

		// 读取 <up> 元素
		XMLElement* upElement = cameraElement->FirstChildElement("up");
		vec3 up = vec3(
			upElement->FloatAttribute("x"),
			upElement->FloatAttribute("y"),
			upElement->FloatAttribute("z")
		);

		Film* filmPtr = new Film();
		if (!test)
			filmPtr->reset(width, height);
		Camera cam(eye, lookat, up, fovy);
		cam.filmPtr = filmPtr;
		this->cam = cam;
	}

	// 读取光照数据
	//XMLElement* lightElement = doc.FirstChildElement("light");
	//if (lightElement) {
	//    const char* radiance = lightElement->Attribute("radiance");
	//    if (radiance) {
	//        std::stringstream ss(radiance);
	//        ss >> lightRadiance.r;
	//        ss.ignore(1, ',');  // 跳过逗号
	//        ss >> lightRadiance.g;
	//        ss.ignore(1, ',');
	//        ss >> lightRadiance.b;
	//    }
	//}
}

void Scene::render() {
	auto w = cam.filmPtr->width;
	auto h = cam.filmPtr->height;

	// Logger logger("D:/code/RayTracing/output/dragon.log");
	int count = 0;
#pragma omp parallel for
	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			PayLoad payload = renderPixel(i, j);
			vec3 color = { 0,0,0 };
			if (payload.ishit) color = glm::vec3{ payload.uv, 1 - payload.uv[0] - payload.uv[1] };
			cam.filmPtr->setPixel(i, j, color);

			// 更新已完成的进度
#pragma omp atomic
			count++;

			// 输出当前进度
			if (count % (w * h / 10) == 0) {
				double progress = (static_cast<float>(count) / (w * h)) * 100;
#pragma omp critical
				{
					std::cout << "Progress: " << progress << "%\n";
				}
			}
		}
	}
	std::cout << "Final Progress: 100%" << std::endl;

	// logger.RestoreOriginalBuffers();
	cam.filmPtr->saveToFile("../../output/image.ppm");
}

PayLoad Scene::renderPixel(int x, int y) {
	Ray ray = cam.generateRay({ x, y }, { 0.5, 0.5 });
	float tMin = 0.f;
	float tMax = INFINITY;
	PayLoad payload{};
	for (int i = 0; i < models.size(); i++) {
		models[i]->intersection(ray, payload, tMin, tMax);
	}
	return payload;
}