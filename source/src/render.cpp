#include "render.hpp"

#include "bvh.hpp"
#include "camera.hpp"
#include "film.hpp"
#include "light.hpp"
#include "material.hpp"
#include "model.hpp"
#include "sampler.hpp"
#include "tinyxml2.h"

Render::Render(const string& sceneDir, const string& fileName) {
    model = std::make_shared<Model>();
    this->fileName = fileName;

    using namespace tinyxml2;

    // load xml file
    const string xmlFile = sceneDir + "/" + fileName + ".xml";

    XMLDocument doc;
    if (doc.LoadFile(xmlFile.c_str()) != XML_SUCCESS) {
        std::cerr << "can't load XML file: " << xmlFile << endl;
        return;
    }

    // read camera
    if (XMLElement* cameraElement = doc.FirstChildElement("camera")) {
        int width = cameraElement->IntAttribute("width");
        int height = cameraElement->IntAttribute("height");
        float fovY = cameraElement->FloatAttribute("fovy");

        XMLElement* eyeElement = cameraElement->FirstChildElement("eye");
        auto eye = vec3(
            eyeElement->FloatAttribute("x"), eyeElement->FloatAttribute("y"),
            eyeElement->FloatAttribute("z")
        );

        XMLElement* lookatElement = cameraElement->FirstChildElement("lookat");
        auto lookat = vec3(
            lookatElement->FloatAttribute("x"), lookatElement->FloatAttribute("y"),
            lookatElement->FloatAttribute("z")
        );

        XMLElement* upElement = cameraElement->FirstChildElement("up");
        auto up = vec3(
            upElement->FloatAttribute("x"), upElement->FloatAttribute("y"),
            upElement->FloatAttribute("z")
        );

        film = std::make_shared<Film>(width, height);
        camera = std::make_shared<Camera>(eye, lookat, up, fovY);
    }

    // read light
    for (XMLElement* lightElement = doc.FirstChildElement("light"); lightElement != nullptr;
         lightElement = lightElement->NextSiblingElement("light")) {
        auto light = std::make_shared<Light>();
        if (const char* matName = lightElement->Attribute("mtlname")) {
            light->setMatName(matName);
        }
        if (const char* radiance = lightElement->Attribute("radiance")) {
            std::stringstream ss(radiance);
            vec3 radianceToVec3;
            ss >> radianceToVec3.r;
            ss.ignore(1, ',');
            ss >> radianceToVec3.g;
            ss.ignore(1, ',');
            ss >> radianceToVec3.b;
            light->setRadiance(radianceToVec3);
        }
        model->addLight(light);
    }

    // add model
    model->loadFromFile(sceneDir, fileName);

    BVHBuild();
}

void Render::render() {
    const auto w = film->width;
    const auto h = film->height;
    const int numPixels = w * h;
    int ssp = 0;
    const auto start = std::chrono::high_resolution_clock::now();

    while (++ssp < maxNumSample) {
#pragma omp parallel for
        for (int i = 0; i < numPixels; i++) {
            int y = i / w, x = i % w;
            Ray ray = camera->rayCasting(film, {x, y});
            // vec3 color = rayTest(ray);
            // bvhPtr->intersection(ray, PayLoad{});
            PayLoad payload{};
            const vec3 color = rayTracing(ray, payload, 0);
            film->addToPixel(x, y, color);
        }
        if (ssp % numIter == 0) {
            auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(
                std::chrono::high_resolution_clock::now() - start
            );
            cout << "Sample: " << ssp << " Elapsed time: " << duration.count() << " seconds"
                 << endl;
        }
        if (ssp % numIter == 0) { film->saveToFile(this->fileName, ssp); }
    }
}

void Render::BVHBuild() {
    const auto start = std::chrono::high_resolution_clock::now();
    bvh = std::make_shared<BVH>(model->getMesh());
    const auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::high_resolution_clock::now() - start
    );
    cout << "BVH Build Time: " << duration.count() << " seconds" << endl;
}

bool Render::iterIntersection(const Ray& ray, PayLoad& payload) const {
    bool inter = false;
    for (const auto mesh : model->getMesh()) {
        if (mesh->intersection(ray, payload)) inter = true;
    }
    return inter;
}

bool Render::intersection(const Ray& ray, PayLoad& payload) const {
    return bvh->intersection(ray, payload);
}

vec3 Render::rayTracing(const Ray& wo, PayLoad& currentPayload, int depth) {
    if (depth >= maxDepth) return vec3(0);
    // judge the first intersection
    if (depth == 0 && !intersection(wo, currentPayload)) return vec3(0);
    if (depth == 0 && currentPayload.mesh->material->light)
        return currentPayload.mesh->material->light->getRadiance();

    currentPayload.initBxDFs();
    shared_ptr<BSDF> bsdf = currentPayload.bsdf;
    bool sampleBSDF = bsdf->sampleBSDF(wo.getDir(), currentPayload.normal);

    // sample light
    auto directLight = vec3(0);
    shared_ptr<Sampler> lightSamplerPtr = sampleLight(currentPayload);
    if (!bsdf->perfectSpecular) {
        // add direct light
        if (lightSamplerPtr) {
            float lightPdf = lightSamplerPtr->getPdf();
            vec3 lightDir = lightSamplerPtr->getDir();
            float lightWeight = powerHeuristic(
                lightPdf, bsdf->accumPdf(wo.getDir(), lightDir, currentPayload.normal)
            );
            if (lightWeight > EPSILON) {
                vec3 LightEmission = lightSamplerPtr->getMeshPtr()->material->light->getRadiance();
                vec3 lightEval = bsdf->accumEval(wo.getDir(), lightDir, currentPayload.normal);
                directLight = LightEmission * lightEval * lightWeight *
                              dot(lightDir, currentPayload.normal) / lightPdf;
            }
        }
    }

    // sample BSDF
    auto indirectLight = vec3(0);
    if (sampleBSDF) {
        vec3 throughput = bsdf->eval * dot(bsdf->wi, currentPayload.normal) / bsdf->pdf;
        PayLoad nextPayLoad;
        vec3 origin = currentPayload.hitPos + currentPayload.normal * EPSILON;
        if (bool nextHit = intersection(Ray(origin, bsdf->wi), nextPayLoad)) {
            // add indirect light
            if (shared_ptr<Light> light = nextPayLoad.mesh->material->light) {
                indirectLight = throughput * light->getRadiance();
                if (!bsdf->perfectSpecular) {
                    vec3 dist = nextPayLoad.hitPos - origin;
                    float cosTheta = dot(normalize(-dist), nextPayLoad.normal);
                    float lightPdf = dot(dist, dist) / cosTheta / model->calculateLightsArea();
                    float scatWeight = powerHeuristic(bsdf->pdf, lightPdf);
                    indirectLight *= scatWeight;
                }
            } else {
                // Russian Roulette
                if (depth >= 3) {
                    if (genRandomFloat() > rrThreshold) return directLight;
                    throughput /= rrThreshold;
                }
                // continue accumlate indirect light
                indirectLight =
                    throughput *
                    rayTracing(Ray(currentPayload.hitPos, bsdf->wi), nextPayLoad, depth + 1);
            }
        }
    }
    return directLight + indirectLight;
}

vec3 Render::rayTest(const Ray& ray) const {
    PayLoad currentPayload;
    if (!intersection(ray, currentPayload)) return vec3(0);
    const shared_ptr<Material> material = currentPayload.mesh->material;
    if (material->light) return material->light->getRadiance();
    const float cosTheta = -dot(currentPayload.normal, ray.getDir());
    const vec2 texCoord = currentPayload.mesh->getTexCoord(currentPayload.uv);
    vec3 diffuse = material->getDiffuse(texCoord);
    if (cosTheta < EPSILON) return vec3(0);
    // return cosTheta * diffuse;
    return currentPayload.normal;
}

shared_ptr<Sampler> Render::sampleLight(const PayLoad& payload) const {
    auto samplerPtr = std::make_shared<Sampler>();

    // select a light
    const shared_ptr<Light> light = model->randomSelectLight();
    // select a point on the light
    int meshIdx = static_cast<int>(genRandomFloat() * light->getMeshes().size());
    meshIdx = meshIdx % light->getMeshes().size();
    const shared_ptr<Mesh> mesh = light->getMeshes()[meshIdx];
    // sample a point on the light
    const vec3 weights = samplerPtr->weightSamplingOnMesh();
    const vec3 lightPos = mesh->vertices[0].pos * weights.x + mesh->vertices[1].pos * weights.y +
                          mesh->vertices[2].pos * weights.z;

    const vec3 lightDir = normalize(lightPos - payload.hitPos);

    //  the light in back face
    if (dot(lightDir, payload.normal) < EPSILON) return nullptr;

    PayLoad lightPayload;

    // judge the light point is visible
    const vec3 origin = payload.hitPos + payload.normal * EPSILON;
    if (bool isHit = intersection(Ray(origin, lightDir), lightPayload);
        !isHit || dot(-lightDir, lightPayload.normal) < EPSILON)
        return nullptr;
    const float hitDist = glm::length(payload.hitPos - lightPayload.hitPos);
    const float lightDist = glm::length(payload.hitPos - lightPos);
    if (fabs(hitDist - lightDist) > 0.01) return nullptr;

    const float lightPdf = lightDist * lightDist / fabs(dot(lightPayload.normal, -lightDir)) /
                           model->calculateLightsArea();

    samplerPtr->setPdf(lightPdf);
    samplerPtr->setPos(lightDir);
    samplerPtr->setMeshPtr(mesh);
    samplerPtr->setPayloadPtr(std::make_shared<PayLoad>(lightPayload));
    return samplerPtr;
}