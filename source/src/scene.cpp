#include <cmath>
#include <sstream>
#include <random>
#include <chrono>
#include "tinyxml2/tinyxml2.h"
#include "light.hpp"
#include "scene.hpp"

Scene::Scene(const string sceneDir, const string fileName)
{
	modelPtr = std::make_shared<Model>();
	this->fileName = fileName;

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

		filmPtr = std::make_shared<Film>(width, height);
		camPtr = std::make_shared<Camera>(eye, lookat, up, fovy);
	}

	// read light
	for (XMLElement* lightElement = doc.FirstChildElement("light"); lightElement != nullptr; lightElement = lightElement->NextSiblingElement("light"))
	{
		shared_ptr<Light> lightPtr = std::make_shared<Light>();
		const char* matName = lightElement->Attribute("mtlname");
		if (matName)
		{
			lightPtr->setMatName(matName);
		}
		const char* radiance = lightElement->Attribute("radiance");
		if (radiance)
		{
			std::stringstream ss(radiance);
			vec3 radiance;
			ss >> radiance.r;
			ss.ignore(1, ',');
			ss >> radiance.g;
			ss.ignore(1, ',');
			ss >> radiance.b;
			lightPtr->setRadiance(radiance);
		}
		modelPtr->addLight(lightPtr);
	}
	modelPtr->loadFromFile(sceneDir, fileName);
}

void Scene::render()
{
	auto w = filmPtr->width;
	auto h = filmPtr->height;
	int numPixels = w * h;
	int ssp = 0;
	auto start = std::chrono::high_resolution_clock::now();
	while (++ssp < maxNumSample) {
#pragma omp parallel for
		for (int i = 0; i < numPixels; i++)
		{
			int y = i / w, x = i % w;
			Ray ray = camPtr->rayCasting(filmPtr, { x, y });
			//vec3 color = rayTest(ray);
			//bvhPtr->intersection(ray, PayLoad{});
			vec3 color = rayTracing(ray, 0);
			filmPtr->addToPixel(x, y, color);
		}
		if (ssp % 10 == 0) {
			auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start);
			cout << "Sample: " << ssp << " Elapsed time: " << duration.count() << " seconds" << endl;
		}
		if (ssp % 100 == 0) {
			filmPtr->saveToFile(this->fileName, ssp);
		}
	}
}

void Scene::BVHBuild()
{
	auto start = std::chrono::high_resolution_clock::now();
	bvhPtr = std::make_shared<BVH>(modelPtr->getMeshPtrs());
	auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start);
	cout << "BVH Build Time: " << duration.count() << " seconds" << endl;
}

//bool Scene::intersection(const Ray& ray, PayLoad& payload) const
//{
//	bool inter = false;
//	for (std::shared_ptr<Mesh> mptr : modelPtr->getMeshPtrs())
//	{
//		if (mptr->intersection(ray, payload))
//			inter = true;
//	}
//	return inter;
//}

bool Scene::intersection(const Ray& ray, PayLoad& payload) const {
	return bvhPtr->intersection(ray, payload);
}

vec3 Scene::rayTracing(const Ray& wo, int depth) {
	if (depth >= maxDepth) return vec3{ 0 };
	PayLoad payload;
	if (!intersection(wo, payload)) return vec3{ 0 };

	shared_ptr<Material>matPtr = payload.matPtr;
	if (payload.matPtr->lightPtr)
		return matPtr->lightPtr->getRadiance();

	payload.initBSDF();
	payload.bsdfPtr->sampleBSDF(wo.getDir(), payload.normal);

	// sample light
	shared_ptr<Sampler> samplerPtr = sampleLight();
	vec3 lightPos = samplerPtr->getPos();
	vec3 ws_dir = normalize(lightPos - payload.hitPos);
	PayLoad lightPayload;
	vec3 L_dir = vec3(0);
	// judge if the light is visible
	bool isHit = intersection(Ray(payload.hitPos, ws_dir), lightPayload);
	if (isHit) {
		float hitDist = glm::length(payload.hitPos - lightPayload.hitPos);
		float lightDist = glm::length(payload.hitPos - lightPos);
		if (hitDist - lightDist > -EPSILON) {
			L_dir = samplerPtr->getMeshPtr()->matPtr->lightPtr->getRadiance() *
				payload.bsdfPtr->eval *
				dot(ws_dir, payload.normal) *
				dot(-ws_dir, lightPayload.normal) /
				static_cast<float>(std::pow(lightDist, 2)) /
				samplerPtr->getPdf();
		}
	}
	// Russian Roulette
	if (genRandomFloat() > rr) return L_dir;

	// secondary ray
	//vec3 wi_dir = matPtr->sampleDir(wo.getDir(), payload.normal);
	//vec3 f = matPtr->brdf(wo.getDir(), wi_dir, payload.normal);
	//float pdf = matPtr->pdf(wo.getDir(), wi_dir, payload.normal);

	vec3 wi_dir = payload.bsdfPtr->wi_dir;
	if (glm::length(wi_dir) < EPSILON) return L_dir;
	vec3 eval = payload.bsdfPtr->eval;
	float pdf = payload.bsdfPtr->pdf;

	vec3 L_indir = rayTracing(Ray(payload.hitPos, wi_dir), depth + 1) *
		dot(wi_dir, payload.normal) *
		eval /
		pdf /
		rr;
	return L_dir + L_indir;
}

vec3 Scene::rayTest(const Ray& ray) const
{
	PayLoad payload;
	if (!intersection(ray, payload)) return vec3{ 0 };

	shared_ptr<Material>matPtr = payload.matPtr;
	if (payload.matPtr->lightPtr)
		return matPtr->lightPtr->getRadiance();
	float cos_theta = std::max(0.f, -dot(payload.normal, ray.getDir()));
	return cos_theta * matPtr->diffuse;
}

shared_ptr<Sampler> Scene::sampleLight() const
{
	shared_ptr<Sampler> samplerPtr = std::make_shared<Sampler>();
	vector<shared_ptr<Light>> lptrs = modelPtr->getLightPtrs();
	// select a light
	int light_id = static_cast<int>(genRandomFloat() * lptrs.size());
	shared_ptr<Light>lptr = lptrs[light_id];
	float pdf_light = 1.f / lptrs.size();

	// select a mesh
	int mesh_id = static_cast<int>(genRandomFloat() * lptr->getMeshPtrs().size());
	shared_ptr<Mesh> meshPtr = lptr->getMeshPtrs()[mesh_id];
	pdf_light *= 1.f / lptr->getArea();

	// sample a point on the light
	vec3 weights = samplerPtr->sampleWeight();
	vec3 lightPos = meshPtr->vertices[0].pos * weights.x +
		meshPtr->vertices[1].pos * weights.y +
		meshPtr->vertices[2].pos * weights.z;
	vec3 lightNormal = meshPtr->vertices[0].normal * weights.x +
		meshPtr->vertices[1].normal * weights.y +
		meshPtr->vertices[2].normal * weights.z;

	samplerPtr->setPdf(pdf_light);
	samplerPtr->setPos(lightPos);
	samplerPtr->setMeshPtr(meshPtr);

	return samplerPtr;
}