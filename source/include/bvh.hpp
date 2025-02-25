#pragma once

#include "head_include.hpp"
#include "bbox.hpp"
#include "model.hpp"

class BVH {
public:

	BVH() {
		left = nullptr;
		right = nullptr;
		bbox = BBOX();
	}

	BVH(const Model& model, vector<shared_ptr<Mesh>>& meshPtrs);

	BBOX bbox;
	vector<std::shared_ptr<Mesh>> meshPtrs;
	BVH* left;
	BVH* right;
};