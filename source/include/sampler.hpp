#pragma once

#include "head_include.hpp"
#include "model.hpp"

class Sampler {
public:
	Sampler() : pdf(0), dir(0) {}

	Sampler(float p, vec3 pos, shared_ptr<Mesh> m) : pdf(p), dir(pos), meshPtr(m) {}

	// sample a weight on a mesh
	vec3 weightSamplingOnMesh() const;

	void setPdf(const float &p) { pdf = p; }
	void setPos(const vec3 &p) { dir = p; }
	void setMeshPtr(shared_ptr<Mesh> m) { meshPtr = m; }
	void setPayloadPtr(shared_ptr<PayLoad> p) { payloadPtr = p; }

	float getPdf() const { return pdf; }
	vec3 getDir() const { return dir; }
	shared_ptr<Mesh> getMeshPtr() const { return meshPtr; }
	shared_ptr<PayLoad>getPayloadPtr() const { return payloadPtr; }
	
private:
	float pdf;
	vec3 dir;
	shared_ptr<PayLoad>payloadPtr;
	shared_ptr<Mesh>meshPtr;
};