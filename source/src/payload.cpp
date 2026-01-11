#include "payload.hpp"

#include "Model.hpp"
#include "material.hpp"

void PayLoad::initBxDFs() {
    bsdf = make_shared<BSDF>();
    const shared_ptr<Material> matPtr = mesh->material;
    vec3 specular = matPtr->specular;
    float shininess = matPtr->shininess;
    vec3 diffuse = matPtr->diffuse;
    const vec2 texCoord = mesh->getTexCoord(uv);
    vec3 texRadiance = matPtr->getDiffuse(texCoord);

    if (shininess >= 10000) {
        bsdf->perfectSpecular = true;
        bsdf->bxdfs.push_back(make_shared<MirrorSpecularBRDF>(specular));
    } else {
        if (glm::length(diffuse) > EPSILON)
            bsdf->bxdfs.push_back(make_shared<LambertianDiffuseBRDF>(diffuse));
        if (glm::length(specular) > EPSILON)
            bsdf->bxdfs.push_back(make_shared<PhongSpecularBRDF>(specular, shininess));
        if (matPtr->useTexture) {
            if (shininess > 1.0f)
                bsdf->bxdfs.push_back(make_shared<PhongSpecularBRDF>(texRadiance, shininess));
            else
                bsdf->bxdfs.push_back(make_shared<LambertianDiffuseBRDF>(texRadiance));
        }
    }
    bsdf->generateBSDFWeight();
}