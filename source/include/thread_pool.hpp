#pragma once

#include <vector>
#include <thread>
#include <list>
#include <mutex>

class Task
{
public:
    virtual void run() = 0;
};

class Thread_Poll
{
public:
    static void worker_thread(Thread_Poll *master); // 线程一个静态工作函数
    Thread_Poll(size_t thread_num = 0);
    ~Thread_Poll();

    void add_task(Task *task); // 任务队列添加新任务
    Task *get_task();          // 获取任务队列中的任务

private:
    bool alive;
    std::vector<std::thread> threads; // 线程实例，存储在vector中
    std::list<Task *> tasks;          // 任务队列
    std::mutex lock;                  // 锁
};