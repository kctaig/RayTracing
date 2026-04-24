#include "payload.hpp"

#include "Model.hpp"
#include "material.hpp"

void PayLoad::initBxDFs() {
    bsdf = make_shared<BSDF>();
    const shared_ptr<Material> matPtr = mesh->material;
    vec3 diffuse = matPtr->diffuse;
    vec3 transmittance = matPtr->transmittance;
    float shininess = matPtr->shininess;
    float ior = matPtr->refraIndex;
    const vec2 texCoord = mesh->getTexCoord(uv);
    vec3 texRadiance = matPtr->getDiffuse(texCoord);
    vec3 specular = matPtr->getSpecular(texCoord);

    bool isTransmissive = (ior > 1.0f && glm::length(transmittance) > 0.01f);
    bool isPerfectMirror =
        (!isTransmissive && shininess >= 1000 && glm::length(specular) > MIN_LIGHTING);

    if (isTransmissive) {  // transimissive
        bsdf->perfectSpecular = true;
        // Keep glass as a pure delta lobe to match the perfectSpecular integration path.
        bsdf->bxdfs.emplace_back(make_shared<DielectricSpecularBTDF>(transmittance, ior));
    } else if (isPerfectMirror) {  // mirror reflection
        bsdf->perfectSpecular = true;
        bsdf->bxdfs.emplace_back(make_shared<MirrorSpecularBRDF>(specular));
    } else {  // diffuse and glossy
        if (glm::length(diffuse) > MIN_LIGHTING || matPtr->useTexture) {
            if (matPtr->useTexture)
                bsdf->bxdfs.emplace_back(make_shared<LambertianDiffuseBRDF>(texRadiance));
            else
                bsdf->bxdfs.emplace_back(make_shared<LambertianDiffuseBRDF>(diffuse));
        }

        if (shininess > 1.0f && glm::length(specular) > MIN_LIGHTING) {
            bsdf->bxdfs.emplace_back(make_shared<PhongSpecularBRDF>(specular, shininess));
        }
    }
    bsdf->generateBSDFWeight();
}
