#pragma once

#include "utils.hpp"

class BxDF {
  public:
    virtual ~BxDF() = default;
    BxDF(const vec3& r) : radiance(r), weight(1.0f) {};
    virtual float pdf(const vec3& wo, const vec3& wi, const vec3& n) const = 0;
    virtual vec3 eval(const vec3& wo, const vec3& wi, const vec3& n) const = 0;
    virtual vec3 sampleDir(const vec3& wo, const vec3& n, bool frontFace) const = 0;
    virtual bool isDelta() const { return false; }
    vec3 getRadiance() const { return radiance; }
    float getWeight() const { return weight; }

    void setWeight(const float w) { weight = w; }

  protected:
    vec3 radiance;
    float weight;
};

class LambertianDiffuseBRDF : public BxDF {
  public:
    LambertianDiffuseBRDF(const vec3& kd) : BxDF(kd) {}
    float pdf(const vec3& wo, const vec3& wi, const vec3& n) const override;
    vec3 eval(const vec3& wo, const vec3& wi, const vec3& n) const override;
    vec3 sampleDir(const vec3& wo, const vec3& n, bool frontFace) const override;
};

class PhongSpecularBRDF : public BxDF {
  public:
    PhongSpecularBRDF(const vec3& ks, float ns) : BxDF(ks), alpha(ns) {}
    float pdf(const vec3& wo, const vec3& wi, const vec3& n) const override;
    vec3 eval(const vec3& wo, const vec3& wi, const vec3& n) const override;
    vec3 sampleDir(const vec3& wo, const vec3& n, bool frontFace) const override;

  private:
    float alpha;
};

class MirrorSpecularBRDF : public BxDF {
  public:
    MirrorSpecularBRDF(const vec3& ks) : BxDF(ks) {}
    float pdf(const vec3& wo, const vec3& wi, const vec3& n) const override;
    vec3 eval(const vec3& wo, const vec3& wi, const vec3& n) const override;
    vec3 sampleDir(const vec3& wo, const vec3& n, bool frontFace) const override;
    bool isDelta() const override { return true; }
};

class DielectricSpecularBTDF : public BxDF {
  public:
    DielectricSpecularBTDF(const vec3& kt, float ior) : BxDF(kt), ior(ior) {}
    float pdf(const vec3& wo, const vec3& wi, const vec3& n) const override;
    vec3 eval(const vec3& wo, const vec3& wi, const vec3& n) const override;
    vec3 sampleDir(const vec3& wo, const vec3& n, bool frontFace) const override;
    bool isDelta() const override { return true; }

  private:
    float ior;
    vec3 reflect(const vec3& wo, const vec3& n) const;
    vec3 refract(const vec3& wo, const vec3& n, float eta) const;
    float fresnel(const vec3& wo, const vec3& n, float eta) const;
};

class BSDF {
  public:
    void generateBSDFWeight();
    bool sampleBSDF(const vec3& wo, const vec3& n, bool frontFace);
    vec3 accumEval(const vec3& wo, const vec3& wi, const vec3& n) const;
    float accumPdf(const vec3& wo, const vec3& wi, const vec3& n) const;
    vec3 sampleDir(const vec3& wo, const vec3& n, bool frontFace) const;

    bool perfectSpecular{false};
    float pdf{0.f};
    vec3 eval{0.f};
    vec3 wi{0.f};
    int sampledBxdfIndex{-1};
    vector<shared_ptr<BxDF>> bxdfs;
};