#include <i686/tasks/Task.hpp>

#include <generic/memory/Memory.hpp>
#include <i686/memory/MemoryMapping.hpp>

#include <i686/tasks/Switch.hpp>
#include <i686/interrupts/Interrupts.hpp>
#include <i686/syscalls/Syscalls.hpp>
#include <i686/tasks/Entry.hpp>

#include "librt/SharedPtr.hpp"
#include "librt/UniquePtr.hpp"
#include <librt/Panic.hpp>

extern "C"
{
  extern char kernel_stack_top[];
  extern char kernel_stack_bottom[];

  extern char kernel_read_only_section_begin [];
  extern char kernel_read_only_section_end [];

  extern char kernel_read_write_section_begin [];
  extern char kernel_read_write_section_end [];
}

namespace core::tasks
{
  static constexpr uintptr_t POISON = 0xDEADBEEF;

  constinit rt::SharedPtr<Task> currentTask; // FIXME: Implement for multiprocessor

  rt::SharedPtr<Task> Task::current()
  {
    return currentTask;
  }

  void Task::makeCurrent(rt::SharedPtr<Task> task)
  {
    if(currentTask && currentTask->state == State::RUNNING)
      currentTask->state = State::RUNNABLE;

    currentTask = task;
    if(task)
    {
      ASSERT(currentTask->state == State::RUNNABLE);
      currentTask->state = State::RUNNING;

      interrupts::setKernelStack(reinterpret_cast<uintptr_t>(task->m_kernelStack.ptr), STACK_SIZE);
      syscalls::setKernelStack(reinterpret_cast<uintptr_t>(task->m_kernelStack.ptr), STACK_SIZE);
      if(task->memoryMapping)
        memory::MemoryMapping::makeCurrent(task->memoryMapping); // Kernel task do not have memory mapping and would use the old one
    }
  }

  void Task::switchTo(rt::SharedPtr<Task> task)
  {
    if(current().get() != nullptr)
    {
      auto previousTask = current();

      makeCurrent(task);
      core_tasks_switch_esp(&previousTask->m_kernelStack.esp, &task->m_kernelStack.esp);
    }
    else
    {
      /*
       * There is no previous task, so we use a dummy esp
       *
       * Note: We cannot call startUserspaceTask ourself, because we have to
       *       switch the stack first. Otherwise, all hell may break loose if we
       *       receive a interrupt after we enable interrupt but before using the
       *       new stack.
       */
      uintptr_t dummyEsp;
      makeCurrent(task);
      core_tasks_switch_esp(&dummyEsp, &task->m_kernelStack.esp);
      __builtin_unreachable();
    }
  }

  namespace
  {
    static constinit std::atomic<pid_t> nextPid = 0;
  }

  rt::SharedPtr<Task> Task::allocate()
  {
    void* kernelStackPage = memory::allocPages(STACK_PAGES_COUNT);
    if(!kernelStackPage)
      return nullptr;

    auto stack = Stack{
      .ptr = kernelStackPage,
      .esp = reinterpret_cast<uintptr_t>(kernelStackPage) + STACK_SIZE
    };

    return rt::makeShared<Task>(nextPid++, stack);
  }

  Task::Task(pid_t pid, Stack kernelStack)
    : pid(pid), m_kernelStack(kernelStack) {}

  Task::~Task()
  {
    memory::freePages(m_kernelStack.ptr, STACK_PAGES_COUNT);
  }

  void Task::kill(status_t status)
  {
    this->state = State::DEAD;
    this->status = status;
  }

  Result<void> Task::asKernelTask(void(*kernelTask)())
  {
    m_kernelStack.esp -= sizeof(uintptr_t);
    *reinterpret_cast<uintptr_t*>(m_kernelStack.esp) = POISON; // This can really be any value, we are just simulating eip pushed when executing a call instruction

    m_kernelStack.esp -= sizeof(uintptr_t);
    *reinterpret_cast<uintptr_t*>(m_kernelStack.esp) = reinterpret_cast<uintptr_t>(kernelTask);

    return {};
  }

  Result<void> Task::asUserspaceTask(uintptr_t entry)
  {
    void(*startUserspaceTask)(uintptr_t) = [](uintptr_t entry){ core_tasks_entry(reinterpret_cast<uintptr_t>(entry)); };

    m_kernelStack.esp -= sizeof(uintptr_t);
    *reinterpret_cast<uintptr_t*>(m_kernelStack.esp) = entry;

    m_kernelStack.esp -= sizeof(uintptr_t);
    *reinterpret_cast<uintptr_t*>(m_kernelStack.esp) = POISON; // This can really be any value, we are just simulating eip pushed when executing a call instruction

    m_kernelStack.esp -= sizeof(uintptr_t);
    *reinterpret_cast<uintptr_t*>(m_kernelStack.esp) = reinterpret_cast<uintptr_t>(startUserspaceTask);

    ASSERT(!memoryMapping);
    memoryMapping = memory::MemoryMapping::allocate();
    if(!memoryMapping)
      return ErrorCode::OUT_OF_MEMORY;

    return {};
  }

}
