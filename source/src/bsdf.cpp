#include "bsdf.hpp"

float LambertianDiffuseBRDF::pdf(const vec3& wo, const vec3& wi, const vec3& n) const {
    (void)wo;

    if (dot(n, wi) < 0.f) return 0.f;
    return 1.0f / M_PI;
}

vec3 LambertianDiffuseBRDF::eval(const vec3& wo, const vec3& wi, const vec3& n) const {
    (void)wi;
    (void)wo;
    (void)n;

    return radiance / M_PI;
}

vec3 LambertianDiffuseBRDF::sampleDir(const vec3& wo, const vec3& n, bool frontFace) const {
    // float u = genRandomFloat();
    // float v = genRandomFloat();
    //// float z = std::fabs(1.0f - 2.0f * u);
    // float z = std::sqrt(1- u * u);
    // float r = std::sqrt(1.0f - z * z);
    // float phi = 2.0f * M_PI * v;
    // vec3 localDir = vec3(r * cos(phi), r * sin(phi), z);
    // return toWorld(localDir, n);

    (void)wo;
    (void)frontFace;

    // reference to PBRT
    vec2 disk;
    const vec2 offset = 2.f * vec2(genRandomFloat(), genRandomFloat()) - vec2(1, 1);
    if (offset.x == 0 && offset.y == 0) disk = vec2(0);
    float theta, r;
    if (abs(offset.x) > abs(offset.y)) {
        r = offset.x;
        theta = M_PI / 4.f * (offset.y / offset.x);
    } else {
        r = offset.y;
        theta = M_PI / 2.f - M_PI / 4.f * (offset.x / offset.y);
    }
    disk = r * vec2(cos(theta), sin(theta));
    const float z = sqrt(1 - disk.x * disk.x - disk.y * disk.y);
    const auto localDir = vec3(disk.x, disk.y, z);
    return toWorld(localDir, n);
}

float PhongSpecularBRDF::pdf(const vec3& wo, const vec3& wi, const vec3& n) const {
    if (dot(n, wi) < 0.f) return 0.f;
    const vec3 h = normalize(wi - wo);
    const float nh = dot(n, h);
    return (alpha + 1.0f) * pow(nh, alpha) / (2.0f * M_PI);
}

vec3 PhongSpecularBRDF::eval(const vec3& wo, const vec3& wi, const vec3& n) const {
    const vec3 h = normalize(wi - wo);
    const float nh = dot(h, n);
    const float spec = pow(nh, alpha);
    const float normFactor = (alpha + 2.0f) / (2.0f * M_PI);
    return radiance * spec * normFactor;
}

vec3 PhongSpecularBRDF::sampleDir(const vec3& wo, const vec3& n, bool frontFace) const {
    (void)frontFace;

    const float u = genRandomFloat();
    const float v = genRandomFloat();
    const float phi = 2.0f * M_PI * u;
    const float cosTheta = pow(v, 1.0f / (alpha + 1.0f));
    const float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
    auto h = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
    h = toWorld(h, n);
    const vec3 wi = wo - 2.0f * dot(wo, h) * h;
    return normalize(wi);
}

float MirrorSpecularBRDF::pdf(const vec3& wo, const vec3& wi, const vec3& n) const {
    (void)wo;

    if (dot(wi, n) < 0.f) return 0.f;
    return 1.0f;
}

vec3 MirrorSpecularBRDF::eval(const vec3& wo, const vec3& wi, const vec3& n) const {
    (void)wo;

    const float cosTheta = dot(wi, n);
    const vec3 fresnelSchlick = radiance + (vec3(1.0f) - radiance) * pow(1.0f - cosTheta, 5.0f);
    return fresnelSchlick / cosTheta;
}

vec3 MirrorSpecularBRDF::sampleDir(const vec3& wo, const vec3& n, bool frontFace) const {
    (void)frontFace;

    return normalize(wo - 2.0f * dot(wo, n) * n);
}

vec3 DielectricSpecularBTDF::reflect(const vec3& wo, const vec3& n) const {
    return normalize(wo - 2.0f * dot(wo, n) * n);
}

vec3 DielectricSpecularBTDF::refract(const vec3& wo, const vec3& n, float eta) const {
    float cosThetaI = glm::clamp(-dot(wo, n), -1.0f, 1.0f);
    float sin2ThetaI = 1.0f - cosThetaI * cosThetaI;
    float sin2ThetaT = eta * eta * sin2ThetaI;

    if (sin2ThetaT > 1.0f) { return vec3(0); }

    float cosThetaT = sqrt(1.0f - sin2ThetaT);
    return normalize(eta * wo + (eta * cosThetaI - cosThetaT) * n);
}

float DielectricSpecularBTDF::fresnel(const vec3& wo, const vec3& n, float eta) const {
    // 需要限制，否则浮点误差会影响后面的计算
    float cosThetaI = glm::clamp(-dot(wo, n), -1.0f, 1.0f);
    float sin2ThetaI = 1.0f - cosThetaI * cosThetaI;
    float sin2ThetaT = eta * eta * sin2ThetaI;

    if (sin2ThetaT > 1.0f) { return 1.0f; }

    float cosThetaT = sqrt(1.0f - sin2ThetaT);

    float rParl = (eta * cosThetaI - cosThetaT) / (eta * cosThetaI + cosThetaT);
    float rPerp = (cosThetaI - eta * cosThetaT) / (cosThetaI + eta * cosThetaT);

    return (rParl * rParl + rPerp * rPerp) / 2.0f;
}

float DielectricSpecularBTDF::pdf(const vec3& wo, const vec3& wi, const vec3& n) const {
    (void)wi;
    (void)wo;
    (void)n;

    return 1.0f;
}

vec3 DielectricSpecularBTDF::eval(const vec3& wo, const vec3& wi, const vec3& n) const {
    (void)wo;
    (void)wi;
    (void)n;

    return vec3(0.f);
}

vec3 DielectricSpecularBTDF::sampleDir(const vec3& wo, const vec3& n, bool frontFace) const {
    bool entering = frontFace;
    // `n` is already oriented to oppose `wo` in intersection code, so keep it as-is.
    vec3 normal = n;
    float eta = entering ? 1.0f / ior : ior;

    float fr = glm::clamp(fresnel(wo, normal, eta), 0.0f, 1.0f);
    vec3 refractedDir = refract(wo, normal, eta);

    if (genRandomFloat() >= fr && length(refractedDir) > EPSILON) return refractedDir;

    return reflect(wo, normal);
}

void BSDF::generateBSDFWeight() {
    float sum = 0.f;
    vector<float> luminances;
    for (shared_ptr<BxDF>& bxdfptr : bxdfs) {
        const float luminance = dot(bxdfptr->getRadiance(), cie1931Weights);
        float weight = pow(luminance, 1.0f);
        sum += weight;
        luminances.push_back(weight);
    }
    if (sum == 0.f) return;
    for (int i = 0; i < luminances.size(); i++) { bxdfs[i]->setWeight(luminances[i] / sum); }
}

bool BSDF::sampleBSDF(const vec3& wo, const vec3& n, bool frontFace) {
    if (bxdfs.empty()) return false;
    wi = sampleDir(wo, n, frontFace);
    // specularMirror or transmissive
    if (perfectSpecular && bxdfs.size() == 1) {
        const float cosTheta = std::max(EPSILON, fabs(dot(wi, n)));
        eval = bxdfs[0]->getRadiance() / cosTheta;
        pdf = 1.0f;
    } else {
        eval = accumEval(wo, wi, n);
        pdf = accumPdf(wo, wi, n);
    }
    if (pdf < EPSILON) return false;
    return true;
}

vec3 BSDF::accumEval(const vec3& wo, const vec3& wi, const vec3& n) const {
    auto res = vec3(0);
    for (shared_ptr bxdfptr : bxdfs) { res += bxdfptr->eval(wo, wi, n); }
    return res;
}

float BSDF::accumPdf(const vec3& wo, const vec3& wi, const vec3& n) const {
    float res = 0.f;
    for (shared_ptr bxdfptr : bxdfs) { res += bxdfptr->pdf(wo, wi, n) * bxdfptr->getWeight(); }
    return res;
}

vec3 BSDF::sampleDir(const vec3& wo, const vec3& n, bool frontFace) const {
    std::vector<float> weiPreSum(bxdfs.size());
    for (int i = 0; i < bxdfs.size(); i++) {
        if (i == 0)
            weiPreSum[i] = bxdfs[i]->getWeight();
        else
            weiPreSum[i] = weiPreSum[i - 1] + bxdfs[i]->getWeight();
    }
    const float randomValue = genRandomFloat() * weiPreSum.back();
    int idx = 0;
    while (idx < bxdfs.size() && weiPreSum[idx] < randomValue) { idx++; }
    return bxdfs[idx]->sampleDir(wo, n, frontFace);
}
