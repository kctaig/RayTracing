#include "bvh.hpp"

BVH::BVH(const Model& model, vector<shared_ptr<Mesh>>& meshPtrs)
{
	// 计算包围盒
	for (auto& mesh : meshPtrs)
	{
		bbox.unionMesh(mesh);
	}
	// 如果只有一个面片
	if (meshPtrs.size() == 1)
	{
		left = nullptr;
		right = nullptr;
		this->meshPtrs = meshPtrs;
		return;
	}
	// 如果只有两个面片
	if (meshPtrs.size() == 2)
	{
		left = new BVH(model, vector{ meshPtrs[0] });
		right = new BVH(model, vector{ meshPtrs[1] });
		return;
	}
	// 选择最长的轴
	int axis = 0;
	float max_length = bbox.max.x - bbox.min.x;
	if (bbox.max.y - bbox.min.y > max_length)
	{
		axis = 1;
		max_length = bbox.max.y - bbox.min.y;
	}
	else if (bbox.max.z - bbox.min.z > max_length)
	{
		axis = 2;
		max_length = bbox.max.z - bbox.min.z;
	}
	// 排序 meshPtrs
	switch (axis) {
	case 0:
		std::sort(meshPtrs.begin(), meshPtrs.end(), [](const std::shared_ptr<Mesh>& a, const std::shared_ptr<Mesh>& b) {
			return a->bboxPtr->min.x < b->bboxPtr->min.x;
			});
		break;
	case 1:
		std::sort(meshPtrs.begin(), meshPtrs.end(), [](const std::shared_ptr<Mesh>& a, const std::shared_ptr<Mesh>& b) {
			return a->bboxPtr->min.y < b->bboxPtr->min.y;
			});
		break;
	case 2:
		std::sort(meshPtrs.begin(), meshPtrs.end(), [](const std::shared_ptr<Mesh>& a, const std::shared_ptr<Mesh>& b) {
			return a->bboxPtr->min.z < b->bboxPtr->min.z;
			});
		break;
	}

	// 选择中位数
	vector <shared_ptr<Mesh>> leftMeshes(meshPtrs.begin(), meshPtrs.begin() + meshPtrs.size() / 2);
	vector <shared_ptr<Mesh>> rightMeshes(meshPtrs.begin() + meshPtrs.size() / 2, meshPtrs.end());
	// 递归构建左右子树
	left = new BVH(model, leftMeshes);
	right = new BVH(model, rightMeshes);
}