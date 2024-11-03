#include "head_include.hpp"
#include "camera.hpp"
#include <scene.hpp>
#include "sphere.hpp"
#include <filesystem>

int main()
{
    Scene scene;

    // set camera
    Film film{200, 200};
    // 右手坐标系，看向负Z轴
    Camera cam({ 0, 0, 0 }, { 0, 1, 0 }, { 1, 0, 0 });
    //Camera cam({0, 0, 0.1}, {0, 1, 0}, {0, 0, 0});
    cam.film = &film;
    scene.cam = cam;

    // set objects
    const std::string fileDir = std::filesystem::path("D:/code/RayTracing/models/simple_dragon").string();
    TriMeshModel model(fileDir, "simple_dragon_z.obj");
    //model.print_model_info();
    scene.addModel(model);
    scene.renderScene();

    return 0;
}