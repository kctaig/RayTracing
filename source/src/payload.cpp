#include "payload.hpp"
#include "material.hpp"
#include "Model.hpp"

void PayLoad::initBxDFs()
{
    bsdfPtr = make_shared<BSDF>();
	shared_ptr<Material> matPtr = meshPtr->matPtr;
    vec3 specular = matPtr->specular;
    float shininess = matPtr->shininess;
    vec2 texCoord = meshPtr->getTexCoord(uv);
    vec3 diffuse = matPtr->getDiffuse(texCoord);

    // add diffuseBRDF and SpecularBRDF
    if (shininess >= 10000)
        bsdfPtr->bxdfPtrs.push_back(make_shared<MirrorSpecularBRDF>(specular));
    else {
    bsdfPtr->bxdfPtrs.push_back(make_shared<LambertianBRDF>(diffuse));
    if (glm::length(specular) > EPSILON)
        bsdfPtr->bxdfPtrs.push_back(make_shared<PhongSpecularBRDF>(specular, shininess));
    }
    bsdfPtr->generateWeight();
}