#include "bvh.hpp"

#include <numeric>
#include <stack>

BVH::BVH(const vector<shared_ptr<Mesh>>& meshes) : meshes(meshes) {
    indices.resize(meshes.size());
    std::iota(indices.begin(), indices.end(), 0);

    nodes.reserve(meshes.size() * 2);

    rootIndex = buildNode(0, static_cast<int>(meshes.size()));
}

int BVH::buildNode(const int start, const int end) {
    if (start >= end) return -1;

    Node node;
    node.startIndex = start;
    node.numMeshes = end - start;

    for (int i = start; i < end; i++) { node.bboxPtr->unionMesh(meshes[indices[i]]); }

    // leaf node
    if (end - start <= LEAST_NUM_MESH) {
        node.isLeaf = true;
        nodes.push_back(node);
        return static_cast<int>(nodes.size()) - 1;
    }

    int axis = node.bboxPtr->selectLongAxis();

    std::sort(indices.begin() + start, indices.begin() + end, [&](int a, int b) {
        return meshes[a]->bboxPtr->center()[axis] < meshes[b]->bboxPtr->center()[axis];
    });

    const int mid = start + (end - start) / 2;
    node.leftNode = buildNode(start, mid);
    node.rightNode = buildNode(mid, end);

    nodes.push_back(node);
    return static_cast<int>(nodes.size()) - 1;
}

bool BVH::intersection(const Ray& ray, PayLoad& payload) const {
    if (rootIndex == -1) return false;
    return intersectionNode(rootIndex, ray, payload);
}

bool BVH::intersectionNode(const int nodeIndex, const Ray& ray, PayLoad& payload) const {
    if (nodeIndex == -1) return false;

    const Node& node = nodes[nodeIndex];

    if (!node.bboxPtr->intersection(ray)) return false;

    bool hit = false;
    if (node.isLeaf) {
        for (int i = 0; i < node.numMeshes; i++) {
            const int meshIdx = indices[node.startIndex + i];
            if (const shared_ptr<Mesh> mesh = meshes[meshIdx]; mesh->intersection(ray, payload)) {
                payload.mesh = mesh;
                hit = true;
            }
        }
        return hit;
    }

    if (intersectionNode(node.leftNode, ray, payload)) hit = true;
    if (intersectionNode(node.rightNode, ray, payload)) hit = true;

    return hit;
}
