#include <scene.hpp>
#include <thread_pool.hpp>
#include <filesystem>
#include <cmath>
#include "log.hpp"
#include <mutex>

#define CURRENT_DIR std::filesystem::path(__FILE__).parent_path()

std::mutex mtxScene;

void Scene::renderScene()
{
    auto w = cam.film->width;
    auto h = cam.film->height;
    std::atomic<int> count(0);

#if 1
    //Logger logger("D:/code/RayTracing/output/dragon.ply");
    auto thread_pool = std::make_shared<Thread_Poll>();
    thread_pool->parallel_for(w, h, [&](size_t x, size_t y)
                              {
            PayLoad payload = renderPixel(x, y);
            vec3 color = { 0,0,0 };
            if (payload.ishit) color = glm::vec3{1, 1, 1};
            cam.film->setPixel(x, y, color);
            count++;
            if (count % w == 0) {
                mtxScene.lock();
                //cout << static_cast<float>(count) / (w * h) << endl;
                mtxScene.unlock();
            } });
    thread_pool->wait();
    //logger.RestoreOriginalBuffers();

#elif 0
    Logger logger("D:/code/RayTracing/output/dragon.log");
    for (size_t i = 0; i < w; i++)
    {
        for (size_t j = 0; j < h; j++)
        {
            PayLoad payload = renderPixel(i, j);
            vec3 color = { 0,0,0 };
            if (payload.ishit) color = glm::vec3{ payload.uv, 1 - payload.uv[0] - payload.uv[1] };
            cam.film->setPixel(i, j, color);
            count++;
            /*if (count % w == 0) {
                cout << static_cast<float>(count) / (w * h) << endl;
            }*/
        }
    }
    logger.RestoreOriginalBuffers();
#endif
    
    cam.film->saveToFile(CURRENT_DIR / "../../output/image.ppm");
}

PayLoad Scene::renderPixel(size_t x, size_t y)
{
    Ray ray = cam.generateRay({x, y}, {0.5, 0.5});
    float tMin = 1e-5;
    float tMax = std::numeric_limits<float>::infinity();
    PayLoad payload{};
    for (size_t i = 0; i < models.size(); i++){
        models[i]->intersection(ray, payload, tMin, tMax);
    }

    /* 
    // 与场景中的sphere求交
    for (int i = 0; i < spheres.size(); i++)
    {
        if (spheres[i].hit_sphere(ray))
        {
            color = vec3(0, .3, .2);
            return color;
        }
    }
    */

    return payload;
}
