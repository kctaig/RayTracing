#include <thread_pool.hpp>

void Thread_Poll::worker_thread(Thread_Poll *master)
{
    while (master->alive)
    {
        Task *task = master->get_task();
        if (task != nullptr)
        {
            task->run();
        }
        else {
            std::this_thread::yield();
        }
    }
}

Thread_Poll::Thread_Poll(size_t thread_num)
{
    alive = true;
    // 若线程数为0，则赋值为CPU的线程数
    if (thread_num == 0)
    {
        thread_num = std::thread::hardware_concurrency();
    }
    // 创建线程
    for (size_t i = 0; i < thread_num; i++)
    {
        threads.push_back(std::thread(std::thread(Thread_Poll::worker_thread, this)));
    }
}

Thread_Poll::~Thread_Poll()
{
    while(!tasks.empty()){  // 等待所有任务结束
        std::this_thread::yield(); // 当前线程放弃处理器
    } 
    alive = false;
    //  join(）等待线程执行结束
    for (auto &thread : threads)
    {
        thread.join();
    }
    // 清除所有线程
    threads.clear();
}

void Thread_Poll::add_task(Task *task)
{
    // 进入add_task函数之后会自动获取锁，并且在退出这个函数后释放这个锁
    std::lock_guard<std::mutex> guard(lock);
    tasks.push_back(task);
}

Task *Thread_Poll::get_task()
{
    std::lock_guard<std::mutex> guard(lock);
    if (tasks.empty()) // 判断人任务队列是否为空
    {
        return nullptr;
    }
    Task *task = tasks.front(); // 获取第一个任务
    tasks.pop_front();
    return task;
}
