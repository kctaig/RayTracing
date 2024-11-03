#pragma once

#include <model.hpp>

class TriMeshModel : public Model
{
public:
    TriMeshModel(std::string file_name_dir, std::string file_name);
    void intersection(const Ray& ray, PayLoad& payload, float& tMin, float& tMax) const;
    void modelInfo();

    vector<Mesh>triangles;
};