#include <scene.hpp>
#include <frame_buffer.hpp>
#include <thread_pool.hpp>
#include <cmath>

#define CURRENT_DIR std::filesystem::path(__FILE__).parent_path()

void Scene::render() {

    //std::atomic<int> count(0);

    // 创建一个帧缓冲
    Frame_Buffer *frame_buffer = new Frame_Buffer(this->get_width(), this->get_height());
    // 创建一个线程池
    auto thread_pool = std::make_shared<Thread_Poll>();
    //Thread_Poll thread_pool{};

    // 添加并行任务
    int pixels = this->get_width() * this->get_height();
    thread_pool->parallel_for(this->get_width(), this->get_height(), [&](size_t x, size_t y)->void {

            vec3 color = render_per_pixel(this->cam.get_pos(), x, y, 0, INFINITY);
            frame_buffer->set_pixel(x, y, color);

         /*   count++;
            if (count % this->get_width() == 0) {
                cout << static_cast<float>(count) / (this->get_width() * this->get_height()) << endl;
            }*/

        });
    // 主线程等待所有线程执行完成
    thread_pool->wait();

    frame_buffer->save_to_file(CURRENT_DIR / "../output/image.ppm");
}

vec3 Scene::render_per_pixel(vec3 p,int x,int y, float t_min, float t_max) {

    Ray ray = cam.gen_primary_ray(((float)x + 0.5) / get_width(), ((float)y + 0.5) / get_height(), (float)get_width()/ get_height());

    vec3 unit_dir = normalize(ray.get_dir());
    auto a = 0.5 * (unit_dir.y + 1.0);
    vec3 color = (float)(1.0 - a) * vec3(1.0, 1.0, 1.0) + (float)a * vec3(0.5, 0.7, 1.0);
    
    //vec3 color = vec3(1.0, 0, 0);

    //与场景中的tri_mesh求交
    for (int i = 0; i < tri_models.size(); i++) {
        vector<Vertex> vertices = tri_models[i].get_model().vertices;
        for (int j = 0; j < tri_models[i].get_model().meshes.size(); j++) {
            Mesh m = tri_models[i].get_model().meshes[j];
            // 光线与三角面片的求交
            vec3 v0 = vertices[m.indices[0]].pos;
            vec3 v1 = vertices[m.indices[1]].pos;
            vec3 v2 = vertices[m.indices[2]].pos;
            vec3 E1 = v1 - v0;
            vec3 E2 = v2 - v0;
            vec3 T = ray.get_origin() - v0;
            vec3 D = ray.get_dir();
            vec3 P = cross(D, E2);
            vec3 Q = cross(T, E1);
            float p_e1 = dot(P, E1);
            float t = dot(Q, E2) / p_e1;
            float u = dot(P, T) / p_e1;
            float v = dot(Q, D) / p_e1;
            if (t > 0 && (u >= 0 && u < 1) && (v >=0 && v < 1)) {
                color = vec3(u, v, 1 - u - v);
            }
        }
    }

    //与场景中的sphere求交
    for (int i = 0; i < spheres.size(); i++) {
        if (spheres[i].hit_sphere(ray)) {
            color = vec3(0, 1, 0);
            return color;
        }
    }
    
    return color;
}
