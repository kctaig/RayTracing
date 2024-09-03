#include <iostream>
#include <thread_pool.hpp>
#include "frame_buffer.hpp"

#define CURRENT_DIR std::filesystem::path(__FILE__).parent_path()

class simple_task : public Task {
   public:
    void run() override { std::cout << "hello world" << std::endl; }
};

int main() {
    // 创建一个帧缓冲
    Frame_Buffer frame_buffer(800, 600);
    // 创建一个线程池
    Thread_Poll thread_pool{};
    // 添加并行任务
    thread_pool.parallel_for(200, 100, [&](size_t x, size_t y)->void {
        frame_buffer.set_pixel(x, y, { 0.2,0.5,0.7 }); 
    });
    // 主线程等待所有线程执行完成
    thread_pool.wait();

    frame_buffer.save_to_file(CURRENT_DIR/ "../output/image.ppm");

    return 0;
}