#pragma once

#include "head_include.hpp"

struct Mesh;

class Light
{
public:
	Light() {}

	vector<shared_ptr<Mesh>>meshPtrs;
	string matName;
	vec3 radiance;
	float area = 0.f;
};