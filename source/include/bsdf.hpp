#pragma once

#include "head_include.hpp"

class BxDF {
  public:
    virtual ~BxDF() = default;
    BxDF(const vec3& r) : radiance(r), weight(1.0f) {};
    virtual float pdf(const vec3& wo, const vec3& wi, const vec3& n) const = 0;
    virtual vec3 eval(const vec3& wo, const vec3& wi, const vec3& n) const = 0;
    virtual vec3 sampleDir(const vec3& wo, const vec3& n) const = 0;
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
    vec3 sampleDir(const vec3& wo, const vec3& n) const override;
};

class PhongSpecularBRDF : public BxDF {
  public:
    PhongSpecularBRDF(const vec3& ks, float ns) : BxDF(ks), alpha(ns) {}
    float pdf(const vec3& wo, const vec3& wi, const vec3& n) const override;
    vec3 eval(const vec3& wo, const vec3& wi, const vec3& n) const override;
    vec3 sampleDir(const vec3& wo, const vec3& n) const override;

  private:
    float alpha;
};

class MirrorSpecularBRDF : public BxDF {
  public:
    MirrorSpecularBRDF(const vec3& ks) : BxDF(ks) {}
    float pdf(const vec3& wo, const vec3& wi, const vec3& n) const override;
    vec3 eval(const vec3& wo, const vec3& wi, const vec3& n) const override;
    vec3 sampleDir(const vec3& wo, const vec3& n) const override;
};

class BSDF {
  public:
    void generateBSDFWeight();
    bool sampleBSDF(const vec3& wo, const vec3& n);
    vec3 accumEval(const vec3& wo, const vec3& wi, const vec3& n) const;
    float accumPdf(const vec3& wo, const vec3& wi, const vec3& n) const;
    vec3 selectDir(const vec3& wo, const vec3& n) const;

    bool perfectSpecular;
    float pdf;
    vec3 eval;
    vec3 wi;
    vector<shared_ptr<BxDF>> bxdfs;
};