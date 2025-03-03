#include "bvh.hpp"
#include <stack>

BVH::BVH(const Model& model, const vector<shared_ptr<Mesh>>& meshptrs)
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
	// 选择最长的轴
	int axis = 0;
	float max_length = bboxPtr->max.x - bboxPtr->min.x;
	if (bboxPtr->max.y - bboxPtr->min.y > max_length)
	{
		axis = 1;
		max_length = bboxPtr->max.y - bboxPtr->min.y;
	}
	else if (bboxPtr->max.z - bboxPtr->min.z > max_length)
	{
		axis = 2;
		max_length = bboxPtr->max.z - bboxPtr->min.z;
	}
	// 排序 meshPtrs
	switch (axis) {
	case 0:
		std::sort(meshPtrs.begin(), meshPtrs.end(), [](const std::shared_ptr<Mesh>& a, const std::shared_ptr<Mesh>& b) {
			return a->bboxPtr->center().x < b->bboxPtr->center().x;
			});
		break;
	case 1:
		std::sort(meshPtrs.begin(), meshPtrs.end(), [](const std::shared_ptr<Mesh>& a, const std::shared_ptr<Mesh>& b) {
			return a->bboxPtr->center().y < b->bboxPtr->center().y;
			});
		break;
	case 2:
		std::sort(meshPtrs.begin(), meshPtrs.end(), [](const std::shared_ptr<Mesh>& a, const std::shared_ptr<Mesh>& b) {
			return a->bboxPtr->center().z < b->bboxPtr->center().z;
			});
		break;
	}

	// 选择中位数
	vector <shared_ptr<Mesh>> leftMeshes(meshPtrs.begin(), meshPtrs.begin() + meshPtrs.size() / 2);
	vector <shared_ptr<Mesh>> rightMeshes(meshPtrs.begin() + meshPtrs.size() / 2, meshPtrs.end());

	left = std::make_shared<BVH>(model, leftMeshes);
	right = std::make_shared<BVH>(model, rightMeshes);
}

bool BVH::intersection(const Ray& ray, const shared_ptr<Model> modelPtr, PayLoad& payload)
{
	//// 与该节点的包围盒不相交
	//if (!bboxPtr->intersection(ray)) return false;
	//bool inter = false, leftInter = false, rightInter = false;
	//if (left) leftInter = left->intersection(ray, modelPtr, payload);
	//if (right) rightInter = right->intersection(ray, modelPtr, payload);
	//if (!left && !right) {
	//	PayLoad tempPayload;
	//	for (auto& meshptr : meshPtrs) {
	//		bool isHit = meshptr->intersection(ray, tempPayload, modelPtr);
	//		if (isHit && tempPayload.t < payload.t) {
	//			inter = true;
	//			payload = tempPayload;
	//		}
	//	}
	//}
	//inter = inter || leftInter || rightInter;
	//return inter;

	std::stack<BVH*> stack;
	stack.push(this);
	float rec_t = payload.t;
	bool isHit = false;
	while (!stack.empty()) {
		auto current = stack.top();
		stack.pop();
		if (!current->bboxPtr->intersection(ray)) continue;
		if (!current->left && !current->right) {
			// 叶子节点：检查所有网格
			for (auto& meshptr : current->meshPtrs) {
				if (meshptr->intersection(ray, payload, modelPtr))
					isHit = true;
			}
		}
		else {
			// 非叶子节点：将左右子树压入栈
			if (current->left) stack.push(current->left.get());
			if (current->right) stack.push(current->right.get());
		}
	}
	return isHit;
}