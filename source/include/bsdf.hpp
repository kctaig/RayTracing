#pragma once

#include "head_include.hpp"

class BxDF {
public:
	BxDF(const vec3& r) : radiance(r), weight(1.0f) {};
	virtual float pdf(const vec3& wo, const vec3& wi, const vec3& n) const = 0;
	virtual vec3 eval(const vec3& wo, const vec3& wi, const vec3& n) const = 0;
	virtual vec3 sampleDir(const vec3& wo, const vec3& n) const = 0;
	vec3 getRadiance()const { return radiance; }
	float getWeight()const { return weight; }

	void setWeight(float w) { weight = w; }
protected:
	vec3 radiance;
	float weight;
};

class LambertianBRDF : public BxDF {
public:
	LambertianBRDF(const vec3& kd) :BxDF(kd) {}
	virtual float pdf(const vec3& wo, const vec3& wi, const vec3& n) const override;
	virtual vec3 eval(const vec3& wo, const vec3& wi, const vec3& n) const override;
	virtual vec3 sampleDir(const vec3& wo, const vec3& n) const override;
};

class PhongSpecularBRDF :public BxDF {
public:
	PhongSpecularBRDF(const vec3& ks, float ns) : BxDF(ks), alpha(ns) {}
	virtual float pdf(const vec3& wo, const vec3& wi, const vec3& n) const override;
	virtual vec3 eval(const vec3& wo, const vec3& wi, const vec3& n) const override;
	virtual vec3 sampleDir(const vec3& wo, const vec3& n) const override;
private:
	float alpha;
};

class MirrorSpecularBRDF :public BxDF {
public:
	MirrorSpecularBRDF(const vec3& ks) : BxDF(ks) {}
	virtual float pdf(const vec3& wo, const vec3& wi, const vec3& n) const override;
	virtual vec3 eval(const vec3& wo, const vec3& wi, const vec3& n) const override;
	virtual vec3 sampleDir(const vec3& wo, const vec3& n) const override;
};


class BSDF {
public:
	void generateWeight();
	void sampleBSDF(const vec3& wo, const vec3& n);
	vec3 eval(const vec3& wo, const vec3& wi, const vec3& n) const;
	float pdf(const vec3& wo, const vec3& wi, const vec3& n) const;

	bool perfectSpecular;
	float BSDFpdf;
	vec3 BSDFeval;
	vec3 wi_dir;
	vector<shared_ptr<BxDF>> bxdfPtrs;
};