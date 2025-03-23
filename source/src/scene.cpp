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
			// vec3 color = rayTest(ray);
			// bvhPtr->intersection(ray, PayLoad{});
			vec3 color = rayTracing(ray, PayLoad{}, 0);
			filmPtr->addToPixel(x, y, color);
		}
		if (ssp % 10 == 0) {
			auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - start);
			cout << "Sample: " << ssp << " Elapsed time: " << duration.count() << " seconds" << endl;
		}
		if (ssp % numIter == 0) {
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

bool Scene::iterIntersection(const Ray& ray, PayLoad& payload) const
{
	bool inter = false;
	for (std::shared_ptr<Mesh> mptr : modelPtr->getMeshPtrs())
	{
		if (mptr->intersection(ray, payload))
			inter = true;
	}
	return inter;
}

bool Scene::intersection(const Ray& ray, PayLoad& payload) const {
	return bvhPtr->intersection(ray, payload);
}

vec3 Scene::rayTracing(const Ray& wo, PayLoad& currentPayload, int depth) {
	if (depth >= maxDepth) return vec3(0);
	if (depth == 0 && !intersection(wo, currentPayload)) return vec3(0);
	if (depth == 0 && currentPayload.meshPtr->matPtr->lightPtr) return currentPayload.meshPtr->matPtr->lightPtr->getRadiance();

	currentPayload.initBxDFs();
	shared_ptr<BSDF> bsdfPtr = currentPayload.bsdfPtr;
	bsdfPtr->sampleBSDF(wo.getDir(), currentPayload.normal);

	// sample light
	vec3 directLight = vec3(0);
	//shared_ptr<Sampler> lightSamplerPtr = sampleLight(currentPayload);
	//if (lightSamplerPtr) {
	//	float lightPdf = lightSamplerPtr->getPdf();
	//	vec3 lightDir = lightSamplerPtr->getDir();
	//	float lightWeight = powerHeuristic(lightPdf, bsdfPtr->pdf(wo.getDir(), lightDir, currentPayload.normal));
	//	if (lightWeight > EPSILON) {
	//		vec3 LightEmission = lightSamplerPtr->getMeshPtr()->matPtr->lightPtr->getRadiance();
	//		vec3 lightEval = bsdfPtr->eval(wo.getDir(), lightDir, currentPayload.normal);
	//		directLight += LightEmission *
	//			lightEval *
	//			lightWeight *
	//			dot(lightDir, currentPayload.normal) /
	//			lightPdf;
	//	}
	//}

	// sample BSDF
	vec3 indirectLight = vec3(0);
	if (bsdfPtr->BSDFpdf > EPSILON) {
		vec3 wi_dir = currentPayload.bsdfPtr->wi_dir;
		vec3 throughput = bsdfPtr->BSDFeval *
			dot(wi_dir, currentPayload.normal) /
			bsdfPtr->BSDFpdf;

		PayLoad nextPayLoad;
		vec3 newOrigin = currentPayload.hitPos + currentPayload.normal * EPSILON;
		bool nextHit = intersection(Ray(newOrigin, wi_dir), nextPayLoad);
		if (nextHit) {
			shared_ptr<Light>lightPtr = nextPayLoad.meshPtr->matPtr->lightPtr;
			if (lightPtr) {
				vec3 dist = nextPayLoad.hitPos - currentPayload.hitPos;
				float cosin = dot(normalize(-dist), nextPayLoad.normal);
				float lightPdf = dot(dist, dist) / cosin / lightPtr->getArea();
				if (lightPdf > EPSILON) {
					float scatWeight = powerHeuristic(bsdfPtr->BSDFpdf, lightPdf);
					//directLight += throughput * lightPtr->getRadiance() * scatWeight;
					directLight += throughput * lightPtr->getRadiance();
					return directLight;
				}
			}
			// Russian Roulette
			if (depth >= 3) {
				if (genRandomFloat() > rrThreshold) {
					return directLight;
				}
				throughput /= rrThreshold;
			}
			indirectLight = throughput * rayTracing(Ray(newOrigin, wi_dir), nextPayLoad, depth + 1);
		}
	}
	return directLight + indirectLight;
}

vec3 Scene::rayTest(const Ray& ray) const
{
	PayLoad currentpayload;
	if (!intersection(ray, currentpayload)) return vec3(0);

	shared_ptr<Material>matPtr = currentpayload.meshPtr->matPtr;
	if (matPtr->lightPtr)
		return matPtr->lightPtr->getRadiance();
	float cos_theta = std::max(0.f, -dot(currentpayload.normal, ray.getDir()));
	vec2 texcoord = currentpayload.meshPtr->getTexCoord(currentpayload.uv);
	vec3 diffuse = matPtr->getDiffuse(texcoord);
	return cos_theta * diffuse;
}

shared_ptr<Sampler> Scene::sampleLight(const PayLoad& payload) const
{
	shared_ptr<Sampler> samplerPtr = std::make_shared<Sampler>();
	vector<shared_ptr<Light>> lptrs = modelPtr->getLightPtrs();
	// select a light
	vector<float>areaPreSum(lptrs.size());
	for (int i = 0; i < lptrs.size(); i++)
	{
		areaPreSum[i] = lptrs[i]->getArea();
		if (i > 0) areaPreSum[i] += areaPreSum[i - 1];
	}
	float randomValue = genRandomFloat() * areaPreSum.back();
	int light_id = 0;
	while (light_id < lptrs.size() && randomValue > areaPreSum[light_id]) light_id++;
	shared_ptr<Light> lptr = lptrs[light_id];
	// select a point on the light
	int mesh_id = genRandomFloat() * lptr->getMeshPtrs().size();
	shared_ptr<Mesh> meshPtr = lptr->getMeshPtrs()[mesh_id];
	// sample a point on the light
	vec3 weights = samplerPtr->sampleWeight();
	vec3 lightPos = meshPtr->vertices[0].pos * weights.x +
		meshPtr->vertices[1].pos * weights.y +
		meshPtr->vertices[2].pos * weights.z;

	vec3 ws_dir = normalize(lightPos - payload.hitPos);
	// back face
	if (dot(ws_dir, payload.normal) < 0.f) return nullptr;
	PayLoad lightPayload;

	// judget the light point is visible
	vec3 newOrigin = payload.hitPos + payload.normal * EPSILON;
	bool isHit = intersection(Ray(newOrigin, ws_dir), lightPayload);
	if (!isHit || dot(-ws_dir, lightPayload.normal) < EPSILON) 
		return nullptr;

	float hitDist = glm::length(payload.hitPos - lightPayload.hitPos);
	float lightDist = glm::length(payload.hitPos - lightPos);
	if (fabs(hitDist - lightDist) > EPSILON) 
		return nullptr;

	float lightPdf = lightDist * lightDist /
		(fabs(dot(lightPayload.normal, -ws_dir)) * areaPreSum.back());

	samplerPtr->setPdf(lightPdf);
	samplerPtr->setPos(ws_dir);
	samplerPtr->setMeshPtr(meshPtr);
	samplerPtr->setPayloadPtr(std::make_shared<PayLoad>(lightPayload));
	return samplerPtr;
}