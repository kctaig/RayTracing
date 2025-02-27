#include <cmath>
#include <sstream>
#include <random>
#include <chrono>
#include "tinyxml2/tinyxml2.h"
#include "light.hpp"
#include "scene.hpp"

Scene::Scene(const string sceneDir, const string fileName)
{
	using namespace tinyxml2;

	// 加载 XML 文件
	const string xmlFile = sceneDir + "/" + fileName + ".xml";

	XMLDocument doc;
	if (doc.LoadFile(xmlFile.c_str()) != XML_SUCCESS)
	{
		std::cerr << "can't load XML file: " << xmlFile << endl;
		return;
	}

	// 读取摄像机数据
	XMLElement* cameraElement = doc.FirstChildElement("camera");
	if (cameraElement)
	{
		int width = cameraElement->IntAttribute("width");
		int height = cameraElement->IntAttribute("height");
		float fovy = cameraElement->FloatAttribute("fovy");

		// 读取 <eye> 元素
		XMLElement* eyeElement = cameraElement->FirstChildElement("eye");
		vec3 eye = vec3(
			eyeElement->FloatAttribute("x"),
			eyeElement->FloatAttribute("y"),
			eyeElement->FloatAttribute("z"));

		// 读取 <lookat> 元素
		XMLElement* lookatElement = cameraElement->FirstChildElement("lookat");
		vec3 lookat = vec3(
			lookatElement->FloatAttribute("x"),
			lookatElement->FloatAttribute("y"),
			lookatElement->FloatAttribute("z"));

		// 读取 <up> 元素
		XMLElement* upElement = cameraElement->FirstChildElement("up");
		vec3 up = vec3(
			upElement->FloatAttribute("x"),
			upElement->FloatAttribute("y"),
			upElement->FloatAttribute("z"));

		Film* filmPtr = new Film(width, height);
		Camera cam(eye, lookat, up, fovy);
		cam.filmPtr = filmPtr;
		this->cam = cam;
	}

	// 读取光照数据
	for (XMLElement* lightElement = doc.FirstChildElement("light"); lightElement != nullptr; lightElement = lightElement->NextSiblingElement("light"))
	{
		Light lightData;
		const char* matName = lightElement->Attribute("mtlname");
		if (matName)
		{
			lightData.matName = matName;
		}
		const char* radiance = lightElement->Attribute("radiance");
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
	int ssp = 0;
	vector<glm::ivec2> pixels;
	for (int j = 0; j < h; j++)
	{
		for (int i = 0; i < w; i++)
		{
			pixels.push_back({ i, j });
		}
	}
	auto start = std::chrono::high_resolution_clock::now();
	while (++ssp < maxNumSample) {
#pragma omp parallel for
		for (int i = 0; i < pixels.size(); i++)
		{
			Ray ray = cam.genPrimaryRay({ pixels[i].x, pixels[i].y });
			vec3 color = rayTracing(ray, 0);
			cam.filmPtr->addToPixel(pixels[i].x, pixels[i].y, color);
		}

		if (ssp % 5 == 0) {
			auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start);
			cout << "Sample: " << ssp << " Elapsed time: " << duration.count() << " seconds" << endl;
			cam.filmPtr->saveToFile("../../output/image.ppm", ssp);
		}
	}
}

void Scene::BVHBuild()
{
	auto start = std::chrono::high_resolution_clock::now();
	bvhPtr = std::make_shared<BVH>(*modelPtr, modelPtr->meshPtrs);
	auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start);
	cout << "BVH Build Time: " << duration.count() << " seconds" << endl;
}

bool Scene::intersection(const Ray& ray, PayLoad& payload) const
{
	bool inter = false;
	for (auto meshPtr : modelPtr->meshPtrs)
	{
		vec3 v0 = modelPtr->vertices[meshPtr->indices[0]].pos;
		vec3 v1 = modelPtr->vertices[meshPtr->indices[1]].pos;
		vec3 v2 = modelPtr->vertices[meshPtr->indices[2]].pos;
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

		if (t >= 0.f && u >= 0 && v >= 0 && 1 - u - v >= 0)
		{
			if (t < payload.t)
			{
				inter = true;
				payload.t = t;
				payload.hitPos = ray.at(t);
				payload.uv = { u, v };
				vec3 normal = (
					modelPtr->vertices[meshPtr->indices[0]].normal +
					modelPtr->vertices[meshPtr->indices[1]].normal +
					modelPtr->vertices[meshPtr->indices[2]].normal) / 3.0f;
				payload.normal = normalize(normal);
				payload.matId = meshPtr->matId;
			}
		}
	}
	return inter;
}

bool Scene::intersection(const Ray& ray, const shared_ptr<Model> modelPtr, PayLoad& payload) const {
	return bvhPtr->intersection(ray, modelPtr, payload);
}

vec3 Scene::rayTracing(const Ray& wo, int depth) {
	if (depth >= maxDepth) return vec3{ 0 };
	PayLoad payload;
	if (!intersection(wo, modelPtr, payload)) return vec3{ 0 };

	Material mat = modelPtr->modelMats[payload.matId];

	// 与光源相交直接返回光源辐射度
	if (mat.lightId >= 0)
		return lights[mat.lightId].radiance;

	// 采样光源
	float pdf_light = 1.f;
	vec3 lightPos = sampleLight(modelPtr, lights, pdf_light);
	vec3 ws = normalize(lightPos - payload.hitPos);
	PayLoad lightPayload;
	vec3 L_dir = vec3(0);
	// 判断是否遮挡
	bool isHit = intersection(Ray(payload.hitPos, ws), modelPtr, lightPayload);
	if (isHit) {
		float hitDist = glm::length(payload.hitPos - lightPayload.hitPos);
		float lightDist = glm::length(payload.hitPos - lightPos);
		if (hitDist - lightDist > -EPLISON) {
			L_dir = lights[mat.lightId].radiance *
				dot(ws, payload.normal) *
				dot(-ws, lightPayload.normal) /
				static_cast<float>(std::pow(lightDist, 2)) /
				pdf_light;
		}
	}

	if (genRandomFloat() > rr) return L_dir;

	// 次级光线
	vec3 wi_dir = mat.sampleDir(wo.getDir(), payload.normal);
	vec3 brdf = mat.brdf(wo.getDir(), wi_dir, payload.normal);
	float pdf_scatter = mat.pdf(wo.getDir(), wi_dir, payload.normal);
	Ray wi(payload.hitPos, wi_dir);
	vec3 L_indir = rayTracing(wi, depth + 1) *
		dot(wi_dir, payload.normal) *
		brdf /
		pdf_scatter /
		rr;
	return L_dir + L_indir;
}

vec3 Scene::sampleLight(const shared_ptr<Model>& modelPtr, const vector<Light>& lights, float& pdf_light) {
	// 随机选择光源
	float light_id = genRandomFloat() * lights.size();
	Light light = lights[light_id];
	pdf_light *= 1.f / lights.size();
	// 在光源中选择一个mesh
	float mesh_id = genRandomFloat() * light.meshPtrs.size();
	shared_ptr<Mesh> meshPtr = light.meshPtrs[mesh_id];
	pdf_light *= 1.f / light.area;
	// 在mesh中选择一个点
	vec3 point = meshPtr->sampleMesh(modelPtr);
	return point;
}

vec3 Scene::sampleHemisphere(const vec3& normal) {
	std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	// 随机极坐标生成
	float phi = genRandomFloat() * 2.0f * M_PI;
	float cos_theta = sqrt(genRandomFloat()); // 余弦采样
	float sin_theta = sqrt(1.0f - cos_theta * cos_theta);
	vec3 up = (fabs(normal.z) < 0.999) ? vec3(0, 0, 1) : vec3(1, 0, 0);
	vec3 tangent = normalize(cross(normal, up));
	vec3 bitangent = cross(normal, tangent);
	vec3 sampleDir
		= sin_theta * cos(phi) * tangent + sin_theta * sin(phi) * bitangent + cos_theta * normal;
	return normalize(sampleDir);
}