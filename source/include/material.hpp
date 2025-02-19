#pragma once

#include "head_include.hpp"

class Material
{
public:
	Material() {}

	Material(std::string name, vec3 kd, vec3 ks, vec3 tr, float ns, float ni) : matName(name), Kd(kd), Ks(ks), Tr(tr), Ns(ns), Ni(ni) {}

	std::string matName;
	vec3 Kd;  // 漫反射颜色
	vec3 Ks;  // 镜面反射颜色
	vec3 Tr;  // 透明度
	float Ns; // 高光指数
	float Ni; // 折射率
};