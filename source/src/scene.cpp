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

	// loda xml file
	const string xmlFile = sceneDir + "/" + fileName + ".xml";

	XMLDocument doc;
	if (doc.LoadFile(xmlFile.c_str()) != XML_SUCCESS)
	{
		std::cerr << "can't load XML file: " << xmlFile << endl;
		return;
	}

	// read camera
	XMLElement* cameraElement = doc.FirstChildElement("camera");
	if (cameraElement)
	{
		int width = cameraElement->IntAttribute("width");
		int height = cameraElement->IntAttribute("height");
		float fovy = cameraElement->FloatAttribute("fovy");

		XMLElement* eyeElement = cameraElement->FirstChildElement("eye");
		vec3 eye = vec3(
			eyeElement->FloatAttribute("x"),
			eyeElement->FloatAttribute("y"),
			eyeElement->FloatAttribute("z"));

		XMLElement* lookatElement = cameraElement->FirstChildElement("lookat");
		vec3 lookat = vec3(
			lookatElement->FloatAttribute("x"),
			lookatElement->FloatAttribute("y"),
			lookatElement->FloatAttribute("z"));

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

	// read light
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
	auto start = std::chrono::high_resolution_clock::now();
	while (++ssp < maxNumSample) {
#pragma omp parallel for
		for (int j = 0; j < h; j++)
		{
			for (int i = 0; i < w; i++)
			{
				Ray ray = cam.genPrimaryRay({ i, j });
				PayLoad localPayload;
				//bvhPtr->intersection(ray, modelPtr, localPayload);
				vec3 color = rayTracing(ray, 0);
				cam.filmPtr->addToPixel(i, j, color);
			}
		}

		//for (int i = 0; i < pixels.size(); i++)
		//{
		//	Ray ray = cam.genPrimaryRay({ pixels[i].x, pixels[i].y });
		//	PayLoad localPayload;
		//	//bvhPtr->intersection(ray, modelPtr, localPayload);
		//	vec3 color = rayTracing(ray, 0);
		//	cam.filmPtr->addToPixel(pixels[i].x, pixels[i].y, color);
		//}

		if (ssp % 1 == 0) {
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
		vec3 N = cross(E1, E2);
		if (dot(N, -ray.getDir()) < 0) continue;
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

	if (mat.lightId >= 0)
		return lights[mat.lightId].radiance;

	// sample light
	float pdf_light = 1.f;
	int light_id = -1;
	auto [lightPos, lightNormal] = sampleLight(modelPtr, lights, light_id, pdf_light);
	vec3 ws = normalize(lightPos - payload.hitPos);
	PayLoad lightPayload;
	vec3 L_dir = vec3(0);

	// judge if the light is visible
	bool isHit = intersection(Ray(payload.hitPos, ws), modelPtr, lightPayload);
	if (isHit) {
		float hitDist = glm::length(payload.hitPos - lightPayload.hitPos);
		float lightDist = glm::length(payload.hitPos - lightPos);
		if (std::fabs(hitDist - lightDist) < EPLISON) {
			L_dir = lights[light_id].radiance *
				mat.brdf(wo.getDir(), ws, payload.normal) *
				dot(ws, payload.normal) *
				dot(-ws, lightNormal) /
				static_cast<float>(std::pow(lightDist, 2)) /
				pdf_light;
		}
	}

	if (genRandomFloat() > rr) return L_dir;

	// secondary ray
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

std::tuple< vec3, vec3 > Scene::sampleLight(const shared_ptr<Model>& modelPtr, const vector<Light>& lights, int& light_id, float& pdf_light) {
	// select a light
	light_id = static_cast<int>(genRandomFloat() * lights.size());
	Light light = lights[light_id];
	pdf_light *= 1.f / lights.size();

	// select a mesh
	int mesh_id = static_cast<int>(genRandomFloat() * light.meshPtrs.size());
	shared_ptr<Mesh> meshPtr = light.meshPtrs[mesh_id];
	pdf_light *= 1.f / light.area;

	// sample a point on the light
	vec3 weights = meshPtr->sampleMesh(modelPtr);
	vec3 lightPos = modelPtr->vertices[meshPtr->indices[0]].pos * weights.x +
		modelPtr->vertices[meshPtr->indices[1]].pos * weights.y +
		modelPtr->vertices[meshPtr->indices[2]].pos * weights.z;
	vec3 lightNormal = modelPtr->vertices[meshPtr->indices[0]].normal * weights.x +
		modelPtr->vertices[meshPtr->indices[1]].normal * weights.y +
		modelPtr->vertices[meshPtr->indices[2]].normal * weights.z;
	return { lightPos, lightNormal };
}