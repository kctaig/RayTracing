#include "payload.hpp"
#include "material.hpp"
#include "Model.hpp"

void PayLoad::initBxDFs()
{
    bsdfPtr = make_shared<BSDF>();
	shared_ptr<Material> matPtr = meshPtr->matPtr;
    vec3 specular = matPtr->specular;
    float shininess = matPtr->shininess;
    vec3 diffuse = matPtr->diffuse;
    if (matPtr->useTexture)
    {
        const vec2 texCoord = meshPtr->getTexCoord(uv);
        diffuse = matPtr->texturePtr->value(texCoord);
    }
    // add LambertianBRDF and SpecularBRDF
    bsdfPtr->bxdfPtrs.push_back(make_shared<LambertianBRDF>(diffuse));
    if (glm::length(specular) > EPSILON)
    {
        bsdfPtr->bxdfPtrs.push_back(make_shared<PhongSpecularBRDF>(specular, shininess));
    }
    bsdfPtr->generateWeight();
}