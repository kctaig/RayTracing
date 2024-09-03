#pragma once

#include <vector>
#include <thread>
#include <list>
#include <spin_lock.hpp>
#include <functional>

class Task
{
public:
    virtual void run() = 0;
};

// 图形并行渲染
class Parallel_For_Task : public Task {
public:
    Parallel_For_Task(size_t x, size_t y, const std::function<void(size_t, size_t)>& lambda) :x(x), y(y), lambda(lambda) {}

    void run()override { lambda(x, y); }
private:
    size_t x, y;
    std::function<void(size_t, size_t)> lambda;
};

// 仅支持主线程添加任务
class Thread_Poll
{
public:
    // 线程一个静态工作函数，线程会不断的获取任务并执行
    static void worker_thread(Thread_Poll *master); 
    Thread_Poll(size_t thread_num = 0);
    ~Thread_Poll();
    // 并行的for循环
    void parallel_for(size_t width, size_t height, const std::function<void(size_t, size_t)>& lambda);
    // 主线程等待所有任务执行完成
    void wait() const; 
    // 任务队列添加新任务
    void add_task(Task *task); 
    // 当前线程获取任务队列中的任务
    Task *get_task();          

private:
    std::atomic<int> alive; 
    std::vector<std::thread> threads; // 线程数组
    std::list<Task *> tasks;           // 任务队列
    Spin_Lock spin_lock;
};