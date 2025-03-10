#pragma once

#include "head_include.hpp"
#include "material.hpp"
#include "bsdf.hpp"

class PayLoad
{
public:
	PayLoad() : hitPos(vec3(0)), normal(vec3(0)), uv(vec2(0)), t(FLT_MAX) {}
	void initBSDF();
	float t;
	vec3 hitPos;
	vec3 normal;
	vec2 uv;
	shared_ptr<Material> matPtr;
	shared_ptr<BSDF> bsdfPtr;
};

class Ray
{
public:
	Ray(vec3 pos, vec3 dir) : origin(pos), dir(dir) {}

	vec3 getOrigin() const { return origin; }
	vec3 getDir() const { return dir; }
	vec3 at(float t) const { return origin + dir * t; }

private:
	vec3 origin;
	vec3 dir;
};

inline void PayLoad::initBSDF()
{
	bsdfPtr = make_shared<BSDF>();
	vec3 diffuse = matPtr->diffuse;
	vec3 specular = matPtr->specular;
	float shininess = matPtr->shininess;

	// add LambertianBRDF and SpecularBRDF
	bsdfPtr->bxdfPtrs.push_back(make_shared<LambertianBRDF>(diffuse));
	if (glm::length(specular) > EPSILON)
	{
		bsdfPtr->bxdfPtrs.push_back(make_shared<SpecularBRDF>(specular, shininess));
	}
	bsdfPtr->generateWeight();
}