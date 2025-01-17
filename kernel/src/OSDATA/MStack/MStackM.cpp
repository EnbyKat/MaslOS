#include "MStackM.h"

void PrintMStackTrace(MStack stack[], int64_t size, BasicRenderer* renderer, uint32_t col)
{
    renderer->Println("STACK TRACE:\n", col);
    for (int i = 0; i < size; i++)
    {
        renderer->Print("  > At \"{}\"", stack[(size - i) - 1].name, col);
        renderer->Println(" in file \"{}\"", stack[(size - i) - 1].filename, col);
    }
    renderer->Println();

    // stackframe* stk;
    // asm("mov %%rbp, %0" : "=r"(stk) ::);

    // renderer->Println("STACK TRACE:\n", col);
    // for (int i = 0; i < 10 && stk != NULL; i++)
    // {
    //     renderer->Println("  > At \"{}\"", ConvertHexToString(stk->eip), col);
    //     //renderer->Println(" in file \"{}\"", stack[i].filename, col);
    //     stk = stk->ebp;
    // }
    // renderer->Println();
}

void PrintMStackTrace(MStack stack[], int64_t size)
{
    PrintMStackTrace(stack, size, GlobalRenderer, Colors.white);
}

void AddToMStack(MStack thing)
{
    if (!osData.enableStackTrace)
        return;

    if (osData.stackPointer < 1000)
    {
        osData.stackArr[osData.stackPointer] = thing;
        osData.stackPointer++;
    }
}

void RemoveLastMStack()
{
    if (!osData.enableStackTrace)
        return;

    if (osData.stackPointer > 0)
        osData.stackPointer--;
}


