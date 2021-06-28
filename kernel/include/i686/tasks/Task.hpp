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
  public:
    friend class Scheduler;

  public:
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
    Task(Stack kernelStack, rt::SharedPtr<memory::MemoryMapping> memoryMapping);
    ~Task();

  public:
    constexpr auto& memoryMapping() { return m_memoryMapping; }

  public:

  public:
    // Set up the our stack to match what switchTask expect to found and invoke
    // startUserspaceTask with appropriate arguments.
    void asUserspaceTask(uintptr_t entry);
    [[noreturn]] static void startUserspaceTask(uintptr_t entry);

  private:
    Stack m_kernelStack;
    rt::SharedPtr<memory::MemoryMapping> m_memoryMapping;

  public:
    vfs::FileDescriptors& fileDescriptors() { return m_fileDescriptors; }

  private:
    vfs::FileDescriptors m_fileDescriptors;
  };
}
