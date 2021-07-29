#pragma once

#include <generic/vfs/FileDescriptors.hpp>
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
    static void intialize();

    static rt::SharedPtr<Task>& current();
    static void makeCurrent(rt::SharedPtr<Task> task);

    static void switchTo(rt::SharedPtr<Task> task);

  public:
    static rt::SharedPtr<Task> allocate();
    rt::SharedPtr<Task> clone();

  public:
    Task(pid_t pid, Stack kernelStack);
    ~Task();

  public:
    void kill(status_t status);

  public:
    Result<void> asKernelTask(void(*kernelTask)());
    Result<void> asUserTask(Registers registers);

  public:
    bool isKernelTask() const { return !memoryMapping; }
    bool isUserTask()   const { return memoryMapping; }

  public:
    pid_t pid;
    status_t status;

  public:
    Registers registers;
    rt::SharedPtr<memory::MemoryMapping> memoryMapping;
    rt::SharedPtr<vfs::FileDescriptors> fileDescriptors;
    Stack kernelStack;

  public:
    enum class State { RUNNING, RUNNABLE, DEAD } state = State::RUNNABLE;

  };
}
