#include <cmath>
#include <sstream>
#include "scene.hpp"
#include "log.hpp"
#include "tinyxml2/tinyxml2.h"
#include <light.hpp>

Scene::Scene(const std::string sceneDir, const std::string fileName, bool test)
{
	using namespace tinyxml2;

	// 加载 XML 文件
	const std::string xmlFile = sceneDir + "/" + fileName + ".xml";

	XMLDocument doc;
	if (doc.LoadFile(xmlFile.c_str()) != XML_SUCCESS)
	{
		std::cerr << "can't load XML file: " << xmlFile << endl;
		return;
	}

	// 读取摄像机数据
	XMLElement *cameraElement = doc.FirstChildElement("camera");
	if (cameraElement)
	{
		int width = cameraElement->IntAttribute("width");
		int height = cameraElement->IntAttribute("height");
		float fovy = cameraElement->FloatAttribute("fovy");

		// 读取 <eye> 元素
		XMLElement *eyeElement = cameraElement->FirstChildElement("eye");
		vec3 eye = vec3(
			eyeElement->FloatAttribute("x"),
			eyeElement->FloatAttribute("y"),
			eyeElement->FloatAttribute("z"));

		// 读取 <lookat> 元素
		XMLElement *lookatElement = cameraElement->FirstChildElement("lookat");
		vec3 lookat = vec3(
			lookatElement->FloatAttribute("x"),
			lookatElement->FloatAttribute("y"),
			lookatElement->FloatAttribute("z"));

		// 读取 <up> 元素
		XMLElement *upElement = cameraElement->FirstChildElement("up");
		vec3 up = vec3(
			upElement->FloatAttribute("x"),
			upElement->FloatAttribute("y"),
			upElement->FloatAttribute("z"));

		Film *filmPtr = new Film();
		if (!test)
			filmPtr->reset(width, height);
		Camera cam(eye, lookat, up, fovy);
		cam.filmPtr = filmPtr;
		this->cam = cam;
	}

	// 读取光照数据
	for (XMLElement *lightElement = doc.FirstChildElement("light"); lightElement != nullptr; lightElement = lightElement->NextSiblingElement("light"))
	{
		Light lightData;
		const char *matName = lightElement->Attribute("mtlname");
		if (matName)
		{
			lightData.matName = matName;
		}
		const char *radiance = lightElement->Attribute("radiance");
		if (radiance)
		{
			std::stringstream ss(radiance);
			ss >> lightData.radiance.r;
			ss.ignore(1, ',');
			ss >> lightData.radiance.g;
			ss.ignore(1, ',');
			ss >> lightData.radiance.b;
		}
		lights.push_back(lightData);
	}
}

void Scene::render()
{
	auto w = cam.filmPtr->width;
	auto h = cam.filmPtr->height;

	// Logger logger("D:/code/RayTracing/output/dragon.log");
	int count = 0;
#pragma omp parallel for
	for (int j = 0; j < h; j++)
	{
		for (int i = 0; i < w; i++)
		{
			vec3 color = renderPixel(i, j);
			cam.filmPtr->setPixel(i, j, color);

			// 更新已完成的进度
#pragma omp atomic
			count++;

			// 输出当前进度
			if (count % (w * h / 10) == 0)
			{
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

PayLoad Scene::intersection(const Ray &ray) const
{
	float tMin = 0.0f, tMax = 1e10;
	PayLoad payload;
	for (int j = 0; j < model->triangles.size(); j++)
	{
		Mesh m = model->triangles[j];
		vec3 v0 = model->vertices[m.indices[0]].pos;
		vec3 v1 = model->vertices[m.indices[1]].pos;
		vec3 v2 = model->vertices[m.indices[2]].pos;
		vec3 E1 = v1 - v0;
		vec3 E2 = v2 - v0;
		vec3 T = ray.getOrigin() - v0;
		vec3 D = normalize(ray.getDir());
		vec3 P = cross(D, E2);
		vec3 Q = cross(T, E1);
		float p_e1 = dot(P, E1);
		float t = dot(Q, E2) / p_e1;
		float u = dot(P, T) / p_e1;
		float v = dot(Q, D) / p_e1;

		if (t >= tMin && u >= 0 && v >= 0 && 1 - u - v >= 0)
		{
			if (t < tMax)
			{
				tMax = t;
				payload.ishit = true;
				payload.hitPos = ray.at(t);
				payload.uv = {u, v};
				vec3 normal = (model->vertices[m.indices[0]].normal + model->vertices[m.indices[1]].normal + model->vertices[m.indices[2]].normal) / 3.0f;
				payload.normal = normalize(normal);
				payload.matId = model->triangles[j].matId;
			}
		}
	}
	return payload;
}

vec3 Scene::renderPixel(int x, int y)
{
	Ray ray = cam.generateRay({x, y}, {0.5, 0.5});
	int i = 0;
	vec3 color = {0, 0, 0};
	while (i++ < 1)
	{
		PayLoad payload = intersection(ray);
		if (!payload.ishit)
			break;
		// 计算出射光线

		// 采样光源

		// 更新颜色
		Material mat = model->mats[payload.matId];
		color = mat.Kd * mat.Ks;
	}
	return color;
}