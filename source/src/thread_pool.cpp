#include <thread_pool.hpp>
#include <spin_lock.hpp>
#include <thread>
#include <functional>

void Thread_Poll::worker_thread(Thread_Poll *master)
{
    while (master->alive == 1)
    {
        // 当前线程从线程池中的任务队列中获取任务
        Task *task = master->get_task();
        if (task != nullptr)
        {
            task->run();
            master->pending_task_count--;
        }
        else
        {
            // 当前线程主动让出CPU
            std::this_thread::yield();
        }
    }
}

Thread_Poll::Thread_Poll(size_t thread_num)
{
    // 初始化线程池
    alive = 1;
    pending_task_count = 0;
    // 若线程数为0，则赋值为CPU的线程数
    if (thread_num == 0)
    {
        thread_num = std::thread::hardware_concurrency();
    }
    // 线程池创建线程,并为线程分配工作函数 worker_thread
    // this是指向当前线程池的指针，它将被传递给worker_thread
    for (size_t i = 0; i < thread_num; i++)
    {
        threads.push_back(std::thread(Thread_Poll::worker_thread, this));
    }
}
//! 目前仅支持主线程添加任务，否则会出错
Thread_Poll::~Thread_Poll()
{
    wait();    // 主线程等待所有任务被执行完成
    alive = 0; // 表示线程池销毁

    for (auto &thread : threads)
    {
        //  join(）等待线程执行结束
        thread.join();
    }
    // 清除所有线程
    threads.clear();
}

// 定义并行for循环的任务
class Parallel_For_Task : public Task
{
public:
    Parallel_For_Task(size_t x, size_t y, const std::function<void(size_t, size_t)>& lambda) : x(x), y(y), lambda(lambda) {}

    void run() override { lambda(x, y); }

private:
    size_t x, y;
    // std::function 能够封装任何可以调用的目标，比如函数、lambda 表达式、函数对象等。
    // void(size_t, size_t) 表示这个 std::function 对象可以调用一个接受两个 size_t 参数并且没有返回值的函数。
    std::function<void(size_t, size_t)> lambda;
};

// 主线程添加并行任务
void Thread_Poll::parallel_for(size_t width, size_t height, const std::function<void(size_t, size_t)> &lambda)
{
    Guard guard(spin_lock);
    // 将所有需要并行处理的任务添加到任务队列中去
    for (size_t y = 0; y < height; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            pending_task_count++;
            tasks.push_back(new Parallel_For_Task(x, y, lambda));
        }
    }
}

void Thread_Poll::wait() const
{
    // 当仍有任务时主动让出cpu
    while (pending_task_count > 0)
    {
        std::this_thread::yield();
    }
}

void Thread_Poll::add_task(Task *task)
{
    // 进入函数后获取锁，并且函数结束之后自动释放
    Guard guard(spin_lock);
    pending_task_count++;
    tasks.push_back(task);
}

Task *Thread_Poll::get_task()
{
    Guard guard(spin_lock);
    // 判断人任务队列是否为空
    if (tasks.empty())
    {
        return nullptr;
    }
    Task *task = tasks.front(); // 获取第一个任务
    tasks.pop_front();
    return task;
}
