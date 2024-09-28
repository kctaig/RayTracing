#include "head_include.hpp"
#include "camera.hpp"
#include <scene.hpp>
#include "sphere.hpp"

int main()
{

    Scene scene;

    // set image plane
    scene.set_width(200);
    scene.set_height(100);

    // set camera
    vec3 cam_pos = {0, 0, 1};
    vec3 up = {0, 1, 0};
    vec3 right = {1, 0, 0};
    Camera cam(cam_pos, up, right);
    cam.set_vfov(90);
    cam.set_focal_length(1);
    scene.set_cam(cam);

    // set objects
    Tri_Model tri_model;
    const std::string file_dir = std::filesystem::path("D:/code/RayTracing/source/models/cube").string();
    tri_model.make_mesh_by_obj(file_dir, "cube.obj");
    // tri_model.print_model_info();
    scene.add_tri_model(tri_model);

     //Sphere s(vec3(0, 0, -1), 0.5);
     //scene.add_sphere(s);

    scene.render();

    return 0;
}