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
    queue.push(task);
}

bool TaskQueue::drain()
{
    bool ran_task = false;
    while (!queue.empty())
    {
        queue.front()();
        queue.pop();
        ran_task = true;
    }

    return ran_task;
}
