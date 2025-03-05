#include "bvh.hpp"
#include <stack>

BVH::BVH(const vector<shared_ptr<Mesh>>& meshptrs)
{
	meshPtrs = meshptrs;
	bboxPtr = std::make_shared<BBox>();
	for (auto& meshPtr : meshPtrs)
	{
		bboxPtr->unionMesh(meshPtr);
	}
	if (meshPtrs.size() <= numMesh)
	{
		left = nullptr;
		right = nullptr;
		return;
	}
	int axis = selectAxis();
	auto compare = [axis](const std::shared_ptr<Mesh>& a, const std::shared_ptr<Mesh>& b) {
		return a->bboxPtr->center()[axis] < b->bboxPtr->center()[axis];
		};
	std::sort(meshPtrs.begin(), meshPtrs.end(), compare);

	vector <shared_ptr<Mesh>> leftMeshes(meshPtrs.begin(), meshPtrs.begin() + meshPtrs.size() / 2);
	vector <shared_ptr<Mesh>> rightMeshes(meshPtrs.begin() + meshPtrs.size() / 2, meshPtrs.end());

	left = std::make_shared<BVH>(leftMeshes);
	right = std::make_shared<BVH>(rightMeshes);
}

int BVH::selectAxis() const 
{
	int axis = 0;
	float max_length = bboxPtr->getMax().x - bboxPtr->getMin().x;
	if (bboxPtr->getMax().y - bboxPtr->getMin().y > max_length)
	{
		axis = 1;
		max_length = bboxPtr->getMax().y - bboxPtr->getMin().y;
	}
	if (bboxPtr->getMax().z - bboxPtr->getMin().z > max_length)
	{
		axis = 2;
		max_length = bboxPtr->getMax().z - bboxPtr->getMin().z;
	}
	return axis;
}

bool BVH::intersection(const Ray& ray, PayLoad& payload)
{
	if (!bboxPtr->intersection(ray)) return false;

	bool isHit = false;

	if (!left && !right) {
		// 叶子节点：检查所有网格
		for (auto& meshptr : meshPtrs) {
			if (meshptr->intersection(ray, payload))
				isHit = true;
		}
	}
	else {
		// 递归检查左右子树
		if (left && left->intersection(ray, payload))
			isHit = true;
		if (right && right->intersection(ray, payload))
			isHit = true;
	}

	return isHit;

	//std::stack<BVH*> stack;
	//stack.push(this);
	//float rec_t = payload.t;
	//bool isHit = false;
	//while (!stack.empty()) {
	//	auto current = stack.top();
	//	stack.pop();
	//	if (!current->bboxPtr->intersection(ray)) continue;
	//	if (!current->left && !current->right) {
	//		// 叶子节点：检查所有网格
	//		for (auto& meshptr : current->meshPtrs) {
	//			if (meshptr->intersection(ray, payload))
	//				isHit = true;
	//		}
	//	}
	//	else {
	//		// 非叶子节点：将左右子树压入栈
	//		if (current->left) stack.push(current->left.get());
	//		if (current->right) stack.push(current->right.get());
	//	}
	//}
	//return isHit;
}