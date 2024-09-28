#pragma once

#include <model.hpp>

class Tri_Model : public Model
{
public:
    Tri_Model() {};
    bool make_mesh_by_obj(std::string file_name_dir, std::string file_name);
    void print_model_info();

    Model get_model() { return model; }

private:
    Model model;
};