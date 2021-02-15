#include "librt/UniquePtr.hpp"
#include <i686/tasks/Switch.hpp>

#include <generic/memory/Memory.hpp>

#include <i686/tasks/Task.hpp>
#include <i686/interrupts/Interrupts.hpp>
#include <i686/syscalls/Syscalls.hpp>
#include <i686/tasks/Entry.hpp>

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

  constinit Task* currentTask = nullptr; // FIXME: Implement for multiprocessor

  Task* Task::current()
  {
    return currentTask;
  }

  rt::UniquePtr<Task> Task::allocate()
  {
    auto memoryMapping = memory::MemoryMapping::allocate();
    if(!memoryMapping)
      return nullptr;

    auto kernelStackPage = memory::allocMappedPages(1);
    if(!kernelStackPage)
      return nullptr;

    auto stack = Stack{
      .ptr = kernelStackPage->address(),
      .size = kernelStackPage->length(),
      .esp = kernelStackPage->address() + kernelStackPage->length()
    };

    return rt::makeUnique<Task>(stack, rt::move(memoryMapping));
  }

  Task::Task(Stack kernelStack, rt::UniquePtr<memory::MemoryMapping> memoryMapping)
    : m_kernelStack(kernelStack), m_memoryMapping(rt::move(memoryMapping)) {}

  Task::~Task()
  {
    auto kernelStackPage = memory::Pages::from(m_kernelStack.ptr, m_kernelStack.size);
    memory::freeMappedPages(kernelStackPage);
  }


  void Task::makeCurrent()
  {
    currentTask = this;
    interrupts::setKernelStack(m_kernelStack.ptr, m_kernelStack.size);
    syscalls::setKernelStack(m_kernelStack.ptr, m_kernelStack.size);
    m_memoryMapping->makeCurrent();
  }

  void Task::switchTo()
  {
    if(current() != nullptr)
    {
      auto& previousTask = *current();
      makeCurrent();
      core_tasks_switch_esp(&previousTask.m_kernelStack.esp, &m_kernelStack.esp);
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
      makeCurrent();
      core_tasks_switch_esp(&dummyEsp, &m_kernelStack.esp);
      __builtin_unreachable();
    }
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
