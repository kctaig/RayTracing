#pragma once

#include "head_include.hpp"
#include "bbox.hpp"
#include "model.hpp"

#define LEAST_NUM_MESH 4

class BVH
{
public:
	BVH(const vector<shared_ptr<Mesh>> &meshPtrs);

	int buildNode(int start, int end);

	bool intersection(const Ray &ray, PayLoad &payload) const;
	bool intersectionNode(int nodeIndex, const Ray &ray, PayLoad &payload) const;

private:
	struct Node
	{
		Node() : bboxPtr(std::make_shared<BBox>()), leftNode(-1), rightNode(-1), startIndex(-1), numMeshes(0), isLeaf(false) {}

		shared_ptr<BBox> bboxPtr;
		int leftNode;
		int rightNode;
		int startIndex;
		int numMeshes;
		bool isLeaf;
	};

	const vector<shared_ptr<Mesh>> &meshPtrs;
	vector<int> meshIndices;
	vector<Node> nodes;
	int rootIndex = -1;
};