#include "bbox.hpp"

#include "model.hpp"


BBox::BBox(const vector<Vertex>& vertices) {
    min = glm::vec3(FLT_MAX);
    max = glm::vec3(-FLT_MAX);
    for (const Vertex& v : vertices) {
        min = glm::min(min, v.pos);
        max = glm::max(max, v.pos);
    }
}

void BBox::unionMesh(const shared_ptr<Mesh>& mesh) {
    min = glm::min(min, mesh->bboxPtr->min);
    max = glm::max(max, mesh->bboxPtr->max);
}

bool BBox::intersection(const Ray& ray) const {
    const vec3 invDir = 1.0f / ray.getDir();
    const vec3 tmin = (min - ray.getOrigin()) * invDir;
    const vec3 tmax = (max - ray.getOrigin()) * invDir;

    const vec3 t1 = glm::min(tmin, tmax);
    const vec3 t2 = glm::max(tmin, tmax);

    const float tNear = glm::max(t1.x, glm::max(t1.y, t1.z));
    const float tFar = glm::min(t2.x, glm::min(t2.y, t2.z));

    return tNear <= tFar && tFar >= 0.0f;
}

int BBox::selectLongAxis() const {
    int axis = 0;
    float maxLength = max.x - min.x;

    if (max.y - min.y > maxLength) {
        axis = 1;
        maxLength = max.y - min.y;
    }
    if (max.z - min.z > maxLength) { axis = 2; }
    return axis;
}

vec3 BBox::center() const { return vec3{0.5f * (min + max)}; }