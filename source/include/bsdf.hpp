#pragma once

#include "head_include.hpp"

enum BxDFType {
	DEFAULT = 0,
	BSDF_DIFFUSE = 1 << 0,
	BSDF_SPECULAR = 1 << 1,
	BSDF_TRANSMISSION = 1 << 2,
};

class BxDF {
public:
	BxDF(const vec3& r, const BxDFType& t) : radiance(r), type(t), weight(1.0f) {};
	virtual float pdf() const = 0;
	virtual vec3 f(const vec3& wo, const vec3& wi, const vec3& n) const = 0;
	virtual vec3 sampleDir(const vec3& wi, const vec3& n) const = 0;
	vec3 getRadiance()const { return radiance; }
	float getWeight()const { return weight; }

	void setWeight(float w) { weight = w; }
protected:
	vec3 radiance;
	BxDFType type;
	float weight;
};

class LambertianBRDF : public BxDF {
	LambertianBRDF(const vec3& kd) :BxDF(kd, BSDF_DIFFUSE) {}
	virtual float pdf() const override;
	virtual vec3 f(const vec3& wo, const vec3& wi, const vec3& n) const override;
	virtual vec3 sampleDir(const vec3& wi, const vec3& n) const override;
};

class SpecularBRDF :public BxDF {
	SpecularBRDF(const vec3& ks, float ns) : BxDF(ks, BSDF_SPECULAR), shininess(ns) {}
	virtual float pdf() const override;
	virtual vec3 f(const vec3& wo, const vec3& wi, const vec3& n) const override;
	virtual vec3 sampleDir(const vec3& wi, const vec3& n) const override;
private:
	float shininess;
};

class BSDF {
public:
	void generateWeight();
	void sampleBSDF(const vec3& wo, const vec3& n);

	float pdf;
	vec3 f;
	vec3 wi_dir;
	vector<shared_ptr<BxDF>> bxdfPtrs;
};