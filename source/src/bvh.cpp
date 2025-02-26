#include "bvh.hpp"

BVH::BVH(const Model& model, const vector<shared_ptr<Mesh>>& meshptrs) :BVH()
{
	meshPtrs = meshptrs;
	for (auto meshPtr : meshPtrs)
	{
		bboxPtr->unionMesh(meshPtr);
	}
	// 如果只有一个面片
	if (meshPtrs.size() < 5)
	{
		left = nullptr;
		right = nullptr;
		this->meshPtrs = meshPtrs;
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
			return (a->bboxPtr->min.x + a->bboxPtr->max.x) < (b->bboxPtr->min.x + b->bboxPtr->max.x);
			});
		break;
	case 1:
		std::sort(meshPtrs.begin(), meshPtrs.end(), [](const std::shared_ptr<Mesh>& a, const std::shared_ptr<Mesh>& b) {
			return (a->bboxPtr->min.y + a->bboxPtr->max.y) < (b->bboxPtr->min.y + b->bboxPtr->max.y);
			});
		break;
	case 2:
		std::sort(meshPtrs.begin(), meshPtrs.end(), [](const std::shared_ptr<Mesh>& a, const std::shared_ptr<Mesh>& b) {
			return (a->bboxPtr->min.z + a->bboxPtr->max.z) < (b->bboxPtr->min.z + b->bboxPtr->max.z);
			});
		break;
	}

	// 选择中位数
	vector <shared_ptr<Mesh>> leftMeshes(meshPtrs.begin(), meshPtrs.begin() + meshPtrs.size() / 2);
	vector <shared_ptr<Mesh>> rightMeshes(meshPtrs.begin() + meshPtrs.size() / 2, meshPtrs.end());
	// 递归构建左右子树
	left = std::make_shared<BVH>(model, leftMeshes);
	right = std::make_shared<BVH>(model, rightMeshes);

	// 更新包围盒
	//bboxPtr = std::make_shared<BBox>(*(left->bboxPtr), *(right->bboxPtr));
}

bool BVH::intersection(const Ray& ray, const shared_ptr<Model> modelPtr, PayLoad& payload) const
{
	// 与该节点的包围盒不相交
	if (!bboxPtr->intersection(ray)) return false;
	bool inter = false;
	if (!left && !right) {
		for (auto& meshptr : meshPtrs) {
			PayLoad meshPayLoad = meshptr->intersection(ray, modelPtr);
			if (meshPayLoad.t < payload.t) {
				payload = meshPayLoad;
				inter = true;
			}
		}
	}
	else {
		// 递归调用左右子树
		bool leftInter = left && left->intersection(ray, modelPtr, payload);
		bool rightInter = right && right->intersection(ray, modelPtr, payload);
		inter = leftInter || rightInter;
	}
	return inter;
}