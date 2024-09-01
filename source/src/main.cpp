#include <iostream>
#include <glm/glm.hpp>
#include <thread_pool.hpp>

class simple_task : public Task
{
public:
    void run() override
    {
        std::cout << "hello world" << std::endl;
    }
};

int main()
{
    Thread_Poll thread_poll{};
    thread_poll.add_task(new simple_task);
    thread_poll.add_task(new simple_task);
    thread_poll.add_task(new simple_task);
    thread_poll.add_task(new simple_task);
    thread_poll.add_task(new simple_task);
    return 0;
}