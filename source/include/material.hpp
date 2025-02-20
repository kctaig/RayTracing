#pragma once

#include "head_include.hpp"

class Material
{
public:
	Material() {}

	Material(string name, vec3 kd, vec3 ks, vec3 tr, float ns, float ni) : matName(name), diffuse(kd), specular(ks), transparency(tr), shininess(ns), refraIndex(ni) {}

	string matName;
	vec3 diffuse;  // 漫反射颜色，不为0则表示有漫反射部分
	vec3 specular;  // 镜面反射颜色，不为0则表示有镜面反射部分
	vec3 transparency;  // 透明度，小于1表示透明材质
	float shininess; // 高光指数，非零表示有高光效果
	float refraIndex; // 折射率，不等1表示有折射效果
};