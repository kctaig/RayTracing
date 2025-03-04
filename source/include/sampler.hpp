#pragma once

#include "head_include.hpp"
#include "model.hpp"

class Sampler {
public:
	Sampler() : pdf(0), pos(0) {}

	Sampler(float p, vec3 pos, shared_ptr<Mesh> m) : pdf(p), pos(pos), meshPtr(m) {}

	vec3 sampleWeight() const;

	void setPdf(float p) { pdf = p; }
	void setPos(vec3 p) { pos = p; }
	void setMeshPtr(shared_ptr<Mesh> m) { meshPtr = m; }

	float getPdf() const { return pdf; }
	vec3 getPos() const { return pos; }
	shared_ptr<Mesh> getMeshPtr() const { return meshPtr; }

private:
	float pdf;
	vec3 pos;
	shared_ptr<Mesh>meshPtr;
};