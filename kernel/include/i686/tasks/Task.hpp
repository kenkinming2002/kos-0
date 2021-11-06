#pragma once

#include <generic/vfs/FileDescriptors.hpp>
#include <generic/vfs/CommandQueue.hpp>
#include <generic/tasks/SchedInfo.hpp>
#include <generic/tasks/Signals.hpp>

#include <i686/tasks/Registers.hpp>
#include <i686/memory/MemoryMapping.hpp>

#include <librt/SharedPtr.hpp>

#include <stdint.h>
#include <stddef.h>

namespace core::tasks
{
  struct Stack
  {
  public:
    void* ptr;
    uintptr_t esp;
  };

  class Task : public rt::SharedPtrHook
  {
  private:
    static constexpr size_t STACK_PAGES_COUNT = 1;
    static constexpr size_t STACK_SIZE        =  STACK_PAGES_COUNT * memory::PAGE_SIZE;

  public:
    static void switchTo(Task* oldTask, Task& newTask);

  public:
    static rt::SharedPtr<Task> allocate();
    rt::SharedPtr<Task> clone();

  public:
    Task(Stack kernelStack);
    ~Task();

  public:
    Result<void> asKernelTask(void(*kernelTask)(void*), void* data);
    Result<void> asUserTask(Registers registers);

  public:
    bool isKernelTask() const { return !memoryMapping; }
    bool isUserTask()   const { return memoryMapping; }

  public:
    Registers registers;
    rt::SharedPtr<memory::MemoryMapping> memoryMapping;
    rt::SharedPtr<vfs::FileDescriptors> fileDescriptors;
    Stack kernelStack;

  public:
    vfs::CommandQueue commandQueue;

  public:
    SigInfo sigInfo;
    SchedInfo schedInfo;
  };
}
