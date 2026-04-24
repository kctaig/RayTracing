#include "payload.hpp"

#include "Model.hpp"
#include "material.hpp"

void PayLoad::initBxDFs() {
    bsdf = make_shared<BSDF>();
    const shared_ptr<Material> matPtr = mesh->material;
    vec3 diffuse = matPtr->diffuse;
    vec3 specular = matPtr->specular;
    vec3 transmittance = matPtr->transmittance;
    float shininess = matPtr->shininess;
    float ior = matPtr->refraIndex;
    const vec2 texCoord = mesh->getTexCoord(uv);
    vec3 texRadiance = matPtr->getDiffuse(texCoord);

    bool isTransmissive =
        (ior > 1.0f && glm::length(transmittance) > 0.01f &&
         glm::length(diffuse) < 0.1f);  // 透明材质
    bool isPerfectMirror =
        (!isTransmissive && shininess >= 1000 && glm::length(specular) > MIN_LIGHTING);
    bool isGlossyOrDiffuse = !isTransmissive && !isPerfectMirror;

    if (isTransmissive) {
        // 透明材质：折射 + 反射（菲涅尔自动分配）
        bsdf->perfectSpecular = true;
        bsdf->bxdfs.emplace_back(make_shared<DielectricSpecularBTDF>(transmittance, ior));

        // 可选：透明材质也可以有微弱的漫反射（如磨砂玻璃）
        if (glm::length(diffuse) > MIN_LIGHTING) {
            bsdf->bxdfs.emplace_back(make_shared<LambertianDiffuseBRDF>(diffuse * 0.5f));
        }
    } else if (isPerfectMirror) {
        // 完美镜面：如镜子、高抛光金属
        bsdf->perfectSpecular = true;
        bsdf->bxdfs.emplace_back(make_shared<MirrorSpecularBRDF>(specular));
    } else {
        // 普通不透明材质：漫反射 + Phong高光
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