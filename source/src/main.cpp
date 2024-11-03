#include "head_include.hpp"
#include "camera.hpp"
#include <scene.hpp>
#include "sphere.hpp"
#include <filesystem>

int main()
{
    Scene scene;

    // set camera
    Film film{50, 50};
    // right coordinates
    Camera cam({0, 0, 0}, {0, 1, 0}, {1, 0, 0});
    cam.film = &film;
    scene.cam = cam;

    // set objects
    const std::string fileDir = std::filesystem::path("D:/code/RayTracing/models/simple_dragon").string();
    TriMeshModel triModel(fileDir, "simple_dragon_z.obj");
    // triModel.modelInfo();
    scene.addModel(&triModel);
    scene.renderScene();

    return 0;
}