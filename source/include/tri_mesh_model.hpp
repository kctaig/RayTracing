#pragma once

#include <model.hpp>

class TriMeshModel : public Model
{
public:
    TriMeshModel(std::string file_name_dir, std::string file_name);
    void print_model_info();

};