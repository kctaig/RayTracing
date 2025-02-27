#include "bvh.hpp"

BVH::BVH(const Model& model, const vector<shared_ptr<Mesh>>& meshptrs)
{
	meshPtrs = meshptrs;
	bboxPtr = std::make_shared<BBox>();
	for (auto& meshPtr : meshPtrs)
	{
		bboxPtr->unionMesh(meshPtr);
	}
	if (meshPtrs.size() < numMesh)
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

	// 并行递归构建左右子树
#pragma omp parallel sections
	{
#pragma omp section
		left = std::make_shared<BVH>(model, leftMeshes);
#pragma omp section
		right = std::make_shared<BVH>(model, rightMeshes);
	}
}

bool BVH::intersection(const Ray& ray, const shared_ptr<Model> modelPtr, PayLoad& payload) const
{
	// 与该节点的包围盒不相交
	if (!bboxPtr->intersection(ray)) return false;
	bool inter = false;
	if (!left && !right) {
		for (auto& meshptr : meshPtrs) {
			inter = inter || meshptr->intersection(ray, payload, modelPtr);
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