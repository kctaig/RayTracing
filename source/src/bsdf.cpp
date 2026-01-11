#include "bsdf.hpp"

float LambertianDiffuseBRDF::pdf(const vec3& wo, const vec3& wi, const vec3& n) const {
    if (dot(n, wi) < 0.f) return 0.f;
    // return dot(n, wi) / M_PI;
    return 1.0f / M_PI;
}

vec3 LambertianDiffuseBRDF::eval(const vec3& wo, const vec3& wi, const vec3& n) const {
    return radiance / M_PI;
}

vec3 LambertianDiffuseBRDF::sampleDir(const vec3& wo, const vec3& n) const {
    // float u = genRandomFloat();
    // float v = genRandomFloat();
    //// float z = std::fabs(1.0f - 2.0f * u);
    // float z = std::sqrt(1- u * u);
    // float r = std::sqrt(1.0f - z * z);
    // float phi = 2.0f * M_PI * v;
    // vec3 localDir = vec3(r * cos(phi), r * sin(phi), z);
    // return toWorld(localDir, n);

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

vec3 PhongSpecularBRDF::sampleDir(const vec3& wo, const vec3& n) const {
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
    if (dot(wi, n) < 0.f) return 0.f;
    return 1.0f;
}

vec3 MirrorSpecularBRDF::eval(const vec3& wo, const vec3& wi, const vec3& n) const {
    const float cosTheta = dot(wi, n);
    const vec3 fresnelSchlick = radiance + (vec3(1.0f) - radiance) * pow(1.0f - cosTheta, 5.0f);
    return fresnelSchlick / cosTheta;
}

vec3 MirrorSpecularBRDF::sampleDir(const vec3& wo, const vec3& n) const {
    return normalize(wo - 2.0f * dot(wo, n) * n);
}

void BSDF::generateBSDFWeight() {
    constexpr vec3 cie1931Weights(0.212671f, 0.715160f, 0.072169f);
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

bool BSDF::sampleBSDF(const vec3& wo, const vec3& n) {
    if (bxdfs.empty()) return false;
    wi = selectDir(wo, n);
    eval = accumEval(wo, wi, n);
    pdf = accumPdf(wo, wi, n);
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

vec3 BSDF::selectDir(const vec3& wo, const vec3& n) const {
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
    return bxdfs[idx]->sampleDir(wo, n);
}
