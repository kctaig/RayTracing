#include "bvh.hpp"
#include <stack>
#include <numeric>

BVH::BVH(const vector<shared_ptr<Mesh>> &meshPtrs) : meshPtrs(meshPtrs)
{
	meshIndices.resize(meshPtrs.size());
	std::iota(meshIndices.begin(), meshIndices.end(), 0);

	nodes.reserve(meshPtrs.size() * 2);

	rootIndex = buildNode(0, static_cast<int>(meshPtrs.size()));
}

int BVH::buildNode(int start, int end)
{
	Node node;
	node.startIndex = start;
	node.numMeshes = end - start;

	for (int i = start; i < end; i++)
	{
		node.bboxPtr->unionMesh(meshPtrs[meshIndices[i]]);
	}

	if (end - start <= LEAST_NUM_MESH)
	{
		node.isLeaf = true;
		nodes.push_back(node);
		return static_cast<int>(nodes.size()) - 1;
	}

	int axis = node.bboxPtr->selectLongAxis();

	std::sort(meshIndices.begin() + start, meshIndices.begin() + end,
			  [&](int a, int b)
			  {
				  return meshPtrs[a]->bboxPtr->center()[axis] <
						 meshPtrs[b]->bboxPtr->center()[axis];
			  });

	int mid = start + (end - start) / 2;
	node.leftNode = buildNode(start, mid);
	node.rightNode = buildNode(mid, end);

	nodes.push_back(node);
	return static_cast<int>(nodes.size()) - 1;
}

bool BVH::intersection(const Ray &ray, PayLoad &payload) const
{
	if (rootIndex == -1)
		return false;
	return intersectionNode(rootIndex, ray, payload);
}

bool BVH::intersectionNode(int nodeIndex, const Ray &ray, PayLoad &payload) const
{
	if (nodeIndex == -1)
		return false;

	const Node &node = nodes[nodeIndex];

	if (!node.bboxPtr->intersection(ray))
		return false;

	bool hit = false;
	if (node.isLeaf)
	{
		for (int i = 0; i < node.numMeshes; i++)
		{
			int meshIdx = meshIndices[node.startIndex + i];
			shared_ptr<Mesh> meshptr = meshPtrs[meshIdx];
			if (meshptr->intersection(ray, payload))
			{
				payload.meshPtr = meshptr;
				hit = true;
			}
		}
		return hit;
	}

	if (intersectionNode(node.leftNode, ray, payload))
		hit = true;
	if (intersectionNode(node.rightNode, ray, payload))
		hit = true;

	return hit;
}
