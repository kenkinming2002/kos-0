#pragma once

#include <generic/vfs/FileDescriptors.hpp>

#include <i686/memory/MemoryMapping.hpp>

#include <librt/containers/Map.hpp>
#include <librt/Optional.hpp>
#include <librt/UniquePtr.hpp>
#include <librt/NonCopyable.hpp>

#include <stdint.h>
#include <stddef.h>


namespace core::tasks
{
  class Task : public rt::NonCopyable
  {
  public:
    friend class Scheduler;

  public:
    struct Stack
    {
    public:
      uintptr_t ptr;
      size_t size;

      uintptr_t esp;
    };

  public:
    static Task* current(); // FIXME: Implement for multiprocessor

  public:
    static rt::UniquePtr<Task> allocate();

  public:
    Task(Stack kernelStack, rt::SharedPtr<memory::MemoryMapping> memoryMapping);
    ~Task();

  public:
    constexpr auto& memoryMapping() { return m_memoryMapping; }

  public:
    void makeCurrent();
    void switchTo();

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
