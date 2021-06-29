#pragma once

#include <generic/vfs/FileDescriptors.hpp>

#include <i686/memory/MemoryMapping.hpp>

#include <librt/SharedPtr.hpp>
#include <librt/containers/Map.hpp>
#include <librt/Optional.hpp>
#include <librt/UniquePtr.hpp>
#include <librt/NonCopyable.hpp>

#include <stdint.h>
#include <stddef.h>


namespace core::tasks
{
  class Task : public rt::SharedPtrHook
  {
  private:
    static constexpr size_t STACK_PAGES_COUNT = 1;
    static constexpr size_t STACK_SIZE        =  STACK_PAGES_COUNT * memory::PAGE_SIZE;
    struct Stack
    {
    public:
      void* ptr;
      uintptr_t esp;
    };

  public:
    static rt::SharedPtr<Task> current();
    static void makeCurrent(rt::SharedPtr<Task> task);
    static void switchTo(rt::SharedPtr<Task> task);

  public:
    static rt::SharedPtr<Task> allocate();

  public:
    Task(pid_t pid, Stack kernelStack);
    ~Task();

  public:
    void kill(status_t status);

  public:
    Result<void> asKernelTask(void(*kernelTask)());
    Result<void> asUserspaceTask(uintptr_t entry);

  public:
    rt::SharedPtr<memory::MemoryMapping> memoryMapping;
    vfs::FileDescriptors fileDescriptors;

    pid_t pid;
    status_t status;
    enum class State { RUNNING, RUNNABLE, DEAD } state = State::RUNNABLE;

  private:
    Stack m_kernelStack;
  };
}
