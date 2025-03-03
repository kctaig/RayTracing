#pragma once

#include "head_include.hpp"
#include "bbox.hpp"
#include "model.hpp"

class BVH {
public:

	BVH() {
		left = nullptr;
		right = nullptr;
		bboxPtr = std::make_shared<BBox>();
	}

	BVH(const Model& model, const vector<shared_ptr<Mesh>>& meshPtrs);
	bool intersection(const Ray& ray, const shared_ptr<Model> modelPtr, PayLoad& payload);
	int numMesh = 5;
	shared_ptr<BBox> bboxPtr;
	vector<std::shared_ptr<Mesh>> meshPtrs;
	shared_ptr<BVH> left;
	shared_ptr<BVH> right;
};