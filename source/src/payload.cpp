#include "payload.hpp"

#include "Model.hpp"
#include "material.hpp"

namespace {
float roughnessToPhongAlpha(float roughness) {
    // Clamp roughness to keep exponent finite and stable.
    const float r = glm::clamp(roughness, 0.02f, 1.0f);
    return std::max(1.0f, 2.0f / (r * r) - 2.0f);
}
}  // namespace

void PayLoad::initBxDFs() {
    bsdf = make_shared<BSDF>();
    const shared_ptr<Material> matPtr = mesh->material;

    const vec2 texCoord = mesh->getTexCoord(uv);

    const vec3 baseColor = matPtr->baseColor.sample(texCoord);
    const vec3 specular = matPtr->specular.sample(texCoord);
    const vec3 transmission = matPtr->transmission.sample(texCoord);
    const float roughness = matPtr->roughness.sample(texCoord);
    const float phongAlpha = roughnessToPhongAlpha(roughness);
    const float ior = std::max(1.0f, matPtr->ior.sample(texCoord));

    const bool hasDiffuse = glm::length(baseColor) > MIN_LIGHTING;
    const bool hasSpecular = glm::length(specular) > MIN_LIGHTING;
    const bool hasTransmission = (ior > 1.0f + 1e-4f && glm::length(transmission) > MIN_LIGHTING);
    const bool useMirrorDelta = hasSpecular && roughness <= 0.02f;

    int deltaLobeCount = 0;

    // Diffuse lobe.
    if (hasDiffuse) { bsdf->bxdfs.emplace_back(make_shared<LambertianDiffuseBRDF>(baseColor)); }

    // Specular lobe: mirror (delta) for very low roughness, otherwise glossy.
    if (hasSpecular && useMirrorDelta) {
        bsdf->bxdfs.emplace_back(make_shared<MirrorSpecularBRDF>(specular));
        deltaLobeCount++;
    }
    if (hasSpecular && !useMirrorDelta) {
        bsdf->bxdfs.emplace_back(make_shared<PhongSpecularBRDF>(specular, phongAlpha));
    }

    // Transmission lobe.
    if (hasTransmission) {
        bsdf->bxdfs.emplace_back(make_shared<DielectricSpecularBTDF>(transmission, ior));
        deltaLobeCount++;
    }

    bsdf->perfectSpecular = (bsdf->bxdfs.size() == 1 && deltaLobeCount == 1);
    bsdf->generateBSDFWeight();
}
