#include "./task_queue.h"

TaskQueue::TaskQueue()
{
}

TaskQueue::~TaskQueue()
{
    drain();
}

void TaskQueue::submit(task_func task)
{
    std::lock_guard<std::mutex> lock(mutex);
    queue.push(std::move(task));
}

bool TaskQueue::drain()
{
    bool ran_task = false;
    while (true)
    {
        task_func task;
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (queue.empty())
            {
                break;
            }

            task = std::move(queue.front());
            queue.pop();
        }

        task();
        ran_task = true;
    }

    return ran_task;
}
