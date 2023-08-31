#ifndef TASK_QUEUE_H_
#define TASK_QUEUE_H_

#include <functional>
#include <queue>

using task_func = typename std::function<void()>;

class TaskQueue
{
    std::queue<task_func> queue;

public:

    TaskQueue();
    virtual ~TaskQueue();

    void submit(task_func task);
    
    // Return true if ran tasks
    bool drain();
};

#endif
