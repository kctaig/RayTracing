#pragma once

#include "head_include.hpp"

class Material
{
public:
	Material() {}

	Material(string name, vec3 kd, vec3 ks, vec3 tr, float ns, float ni) : matName(name), diffuse(kd), specular(ks), transparency(tr), shininess(ns), refraIndex(ni) {}

	float pdf(const vec3 ray_in, const vec3 ray_out, const vec3 normal);
	vec3 brdf(const vec3 ray_in, const vec3 ray_out, const vec3 normal);
	vec3 sampleDir(const vec3 ray_in, const vec3 normal);

	string matName;
	vec3 diffuse;  
	vec3 specular;  
	vec3 transparency;  
	float shininess; 
	float refraIndex; 

	int lightId = -1;
};