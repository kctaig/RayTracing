#include <scene.hpp>
#include <thread_pool.hpp>
#include <filesystem>
#include <cmath>
#include "log.hpp"

#define CURRENT_DIR std::filesystem::path(__FILE__).parent_path()

void Scene::renderScene()
{
    auto w = cam.film->width;
    auto h = cam.film->height;
    cam.generateMatrix();
    std::atomic<int> count(0);

#if 1
    auto thread_pool = std::make_shared<Thread_Poll>();
    thread_pool->parallel_for(w, h, [&](size_t x, size_t y)
                              {
                                vec3 color = renderPixel(x, y);
                                cam.film->setPixel(x, y, color);
                                count++;
                                if (count % w == 0) {
                                    cout << static_cast<float>(count) / (w * h) << endl;
                                } });
    thread_pool->wait();

#elif 0
    Logger logger("D:/code/RayTracing/output/dragon.log");
    for (size_t i = 0; i < w; i++)
    {
        for (size_t j = 0; j < h; j++)
        {
            cout << "pixel: " <<i<<" "<< j << endl;
            vec3 color = renderPixel(i, j);
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

vec3 Scene::renderPixel(size_t x, size_t y)
{
    Ray ray = cam.generateRay({x, y}, {0.5, 0.5});
    float tMin = 0.f;
    float tNear = INFINITY;
    auto color = vec3(0);
    for (size_t i = 0; i < models.size(); i++)
    {
        vector<Vertex> vertices = models[i].vertices;
        vector<Mesh> meshes = models[i].meshes;
        for (size_t j = 0; j < meshes.size(); j++)
        {
            Mesh m = meshes[j];
            // 光线与三角面片的求交
            vec3 v0 = vertices[m.indices[0]].pos;
            vec3 v1 = vertices[m.indices[1]].pos;
            vec3 v2 = vertices[m.indices[2]].pos;
            vec3 E1 = v1 - v0;
            vec3 E2 = v2 - v0;
            vec3 T = ray.get_origin() - v0;
            vec3 D = normalize(ray.get_dir());
            vec3 P = cross(D, E2);
            vec3 Q = cross(T, E1);
            float p_e1 = dot(P, E1);
            float t = dot(Q, E2) / p_e1;
            float u = dot(P, T) / p_e1;
            float v = dot(Q, D) / p_e1;
            
            if (t >= tMin && u >= 0 && v >= 0 && 1 - u - v >= 0)
            {
                //cout <<"valid: " << t << " " << u << " " << v << endl;
                if (t < tNear)
                {
                    tNear = t;
                    color = vec3(1.f,1.f,1.f);
                }
            }
        }
    }

    // 与场景中的sphere求交
    for (int i = 0; i < spheres.size(); i++)
    {
        if (spheres[i].hit_sphere(ray))
        {
            color = vec3(0, .3, .2);
            return color;
        }
    }

    return color;
}
