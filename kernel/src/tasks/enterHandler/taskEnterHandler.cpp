#include "taskEnterHandler.h"
#include "../../cstr.h"
#include "../../panic.h"
#include "../../memory/heap.h"

TaskEnterHandler::TaskEnterHandler(TerminalInstance* terminal)
{
    this->terminal = terminal;
    this->done = false;
    this->type = TaskType::HANDLEENTER;
    magic = 134127;
}

void TaskEnterHandler::Do()
{
    //GlobalRenderer->Println("ADDR: {}", ConvertHexToString((uint64_t)terminal), Colors.bblue);
    if (magic != 134127)
        while(true);//Panic("INVALID TASK EXECUTED");
    if (terminal != NULL)
        terminal->HandleEnter();
    done = true;
}

TaskEnterHandler* NewEnterTask(TerminalInstance* terminal)
{
    TaskEnterHandler* task = (TaskEnterHandler*)malloc(sizeof(TaskEnterHandler));
    *task = TaskEnterHandler(terminal);
    return task;
}

void TaskEnterHandler::Free()
{

}