#include <thread_pool.hpp>
#include <spin_lock.hpp>
#include <thread>
#include <functional>

void Thread_Poll::worker_thread(Thread_Poll* master) {
    while (master->alive == 1) {
        Task *task = master->get_task(); // 当前线程从线程池中的任务队列中获取任务
        if (task != nullptr) {
            task->run();
            master->pending_task_count--;
        } else {
            std::this_thread::yield(); // 当前线程主动让出CPU
        }
    }
}

Thread_Poll::Thread_Poll(size_t thread_num) {
    alive = 1;
    pending_task_count = 0;
    // 若线程数为0，则赋值为CPU的线程数
    if (thread_num == 0) {
        thread_num = std::thread::hardware_concurrency();
    }
    // 线程池创建线程,并为线程分配工作函数 worker_thread
    // this是指向当前线程池的指针，它将被传递给worker_thread
    for (size_t i = 0; i < thread_num; i++) {
        threads.push_back(std::thread(std::thread(Thread_Poll::worker_thread, this)));
    }
}

Thread_Poll::~Thread_Poll(){
    wait(); // 等待所有仍任务被执行完成
    alive = 0; // 不允许线程获取任务
    //  join(）等待线程执行结束
    for (auto& thread : threads) {
        thread.join();
    }
    // 清除所有线程
    threads.clear();
}

// 主线程添加并行任务
void Thread_Poll::parallel_for(size_t width, size_t height, const std::function<void(size_t, size_t)>& lambda) {
    Guard guard(spin_lock);
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            tasks.push_back(new Parallel_For_Task(x, y, lambda));
        }
    }
}

void Thread_Poll::wait() const {
    while (pending_task_count > 0) {
        std::this_thread::yield();
    }
}

void Thread_Poll::add_task(Task *task){
    Guard guard(spin_lock);
    pending_task_count++;
    tasks.push_back(task);
}

Task *Thread_Poll::get_task(){
    Guard guard(spin_lock);
    // 判断人任务队列是否为空
    if (tasks.empty()) {
        return nullptr;
    }
    Task* task = tasks.front();  // 获取第一个任务
    tasks.pop_front();
    return task;
}
