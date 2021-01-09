#pragma once

#include <core/i686/memory/MemoryMapping.hpp>

#include <optional>

#include <stdint.h>
#include <stddef.h>


namespace core::tasks
{
  class Task
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
    static Task* current; // FIXME: Implement for multiprocessor

  public:
    static std::optional<Task> allocate();

  public:
    Task(Stack kernelStack, memory::MemoryMapping memoryMapping);
    ~Task();

  public:
    Task(Task&& other);
    Task& operator=(Task&& other);

  public:
    constexpr auto& memoryMapping() { return m_memoryMapping; }

  public:
    void makeCurrent();

  public:
    // Set up the our stack to match what switchTask expect to found and invoke
    // startUserspaceTask with appropriate arguments.
    void asUserspaceTask(uintptr_t entry);
    [[noreturn]] static void startUserspaceTask(uintptr_t entry);

  private:
    Stack m_kernelStack;
    memory::MemoryMapping m_memoryMapping; // FIXME: Do reference counting
  };
}
