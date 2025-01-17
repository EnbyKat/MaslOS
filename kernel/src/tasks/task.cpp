#include "task.h"
#include "sleep/taskSleep.h"
#include "enterHandler/taskEnterHandler.h"
#include "../memory/heap.h"


bool Task::GetDone()
{
    return done;
}

TaskType Task::GetType()
{
    return type;
}

Task::Task()
{
    done = false;
    this->type = TaskType::NONE;
}



void DoTask(Task* task)
{
    if (task->GetDone())
        return;
    
    switch (task->GetType())
    {
        case TaskType::NONE:
        {
            
            break;
        }
        case TaskType::SLEEP:
        {
            //GlobalRenderer->Println("SLEEP TASK!");
            TaskSleep* sleep = (TaskSleep*)task;
            sleep->Do();
            break;
        }
        case TaskType::HANDLEENTER:
        {
            //GlobalRenderer->Println("ENTER TASK!");
            TaskEnterHandler* enter = (TaskEnterHandler*)task;
            enter->Do();
            break;
        }
    }
}

void FreeTask(Task* task)
{
    switch (task->GetType())
    {
        case TaskType::NONE:
        {

            break;
        }
        case TaskType::SLEEP:
        {
            TaskSleep* sleep = (TaskSleep*)task;
            sleep->Free();
            free((void*)sleep);
            break;
        }
        case TaskType::HANDLEENTER:
        {
            TaskEnterHandler* enter = (TaskEnterHandler*)task;
            enter->Free();
            free((void*)enter);
            break;
        }
    }  
}