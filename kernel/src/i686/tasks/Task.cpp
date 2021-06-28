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
    currentTask = task;
    if(task)
    {
      interrupts::setKernelStack(reinterpret_cast<uintptr_t>(task->m_kernelStack.ptr), STACK_SIZE);
      syscalls::setKernelStack(reinterpret_cast<uintptr_t>(task->m_kernelStack.ptr), STACK_SIZE);
      memory::MemoryMapping::makeCurrent(task->m_memoryMapping);
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



  rt::SharedPtr<Task> Task::allocate()
  {
    auto memoryMapping = memory::MemoryMapping::allocate();
    if(!memoryMapping)
      return nullptr;

    void* kernelStackPage = memory::allocPages(STACK_PAGES_COUNT);
    if(!kernelStackPage)
      return nullptr;

    auto stack = Stack{
      .ptr = kernelStackPage,
      .esp = reinterpret_cast<uintptr_t>(kernelStackPage) + STACK_SIZE
    };

    return rt::makeShared<Task>(stack, rt::move(memoryMapping));
  }

  Task::Task(Stack kernelStack, rt::SharedPtr<memory::MemoryMapping> memoryMapping)
    : m_kernelStack(kernelStack), m_memoryMapping(rt::move(memoryMapping)) {}

  Task::~Task()
  {
    memory::freePages(m_kernelStack.ptr, STACK_PAGES_COUNT);
  }

  void Task::asUserspaceTask(uintptr_t entry)
  {
    m_kernelStack.esp -= sizeof entry;
    *reinterpret_cast<uintptr_t*>(m_kernelStack.esp) = entry;

    m_kernelStack.esp -= sizeof(uintptr_t);
    *reinterpret_cast<uintptr_t*>(m_kernelStack.esp) = POISON; // This can really be any value, we are just simulating eip pushed when executing a call instruction

    m_kernelStack.esp -= sizeof(uintptr_t);
    *reinterpret_cast<uintptr_t*>(m_kernelStack.esp) = reinterpret_cast<uintptr_t>(&startUserspaceTask);
  }

  [[noreturn]] void Task::startUserspaceTask(uintptr_t entry)
  {
    core_tasks_entry(entry);
  }
}
