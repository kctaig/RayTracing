#pragma once

#include "head_include.hpp"
#include "light.hpp"

#include "stb_image.h"

class Texture {

public:
	Texture() = default;
	~Texture() { stbi_image_free(data); }	
	Texture(const string& path, const string& texName);
	vec3 value(const vec2 &texCoord) const;

	float* data;
	int width;
	int height;
	int channels;
};


class Material
{
public:
	Material() = default;

	Material(string name, vec3 kd, vec3 ks, vec3 tr, float ns, float ni) : matName(name), diffuse(kd), specular(ks), transparency(tr), shininess(ns), refraIndex(ni) {}

	vec3 getDiffuse(const vec2& texcoord) const;

	string matName;
	vec3 diffuse;
	vec3 specular;
	vec3 transparency;
	float shininess;
	float refraIndex;
	shared_ptr<Light> lightPtr;

	bool useTexture = false;
	shared_ptr<Texture> texturePtr;
};