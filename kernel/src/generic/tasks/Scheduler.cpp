#include <generic/tasks/Scheduler.hpp>

#include <generic/Init.hpp>
#include <common/generic/io/Print.hpp>
#include <i686/syscalls/Syscalls.hpp>
#include <i686/interrupts/Interrupts.hpp>
#include <x86/interrupts/8259.hpp>
#include <i686/tasks/Switch.hpp>

#include <functional>
#include <assert.h>

namespace core::tasks
{
  INIT_NORMAL Scheduler scheduler;

  /* 
   * Yield syscall 
   *
   * Note: We can just use cli and sti instruction instead of
   *       saving and restoring eflags. This is based on the assumption that
   *       syscall must come from userspace and thus, interrupt must be enabled.
   */
  static int syscallYield(int, int, int, int)
  { 
    asm volatile ("cli" : : : "memory");
    scheduler.schedule(); 
    asm volatile ("sti" : : : "memory");
    return 0;
  }

  /* 
   * Note: We have to disable interrupt, however, locking would not be
   *       necessary, since we would or *WILL* have a per-cpu task queue. 
   */
  static void timerHandler(uint8_t, uint32_t, uintptr_t)
  { 
    interrupts::acknowledge(0); 
    scheduler.schedule(); 
  }

  Scheduler::Scheduler()
  {
    syscalls::installHandler(0, &syscallYield);
    interrupts::installHandler(0x20, &timerHandler, PrivilegeLevel::RING0, true);
    interrupts::clearMask(0);
  }

  [[noreturn]] void Scheduler::startFirstUserspaceTask()
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
    m_tasks.begin()->makeCurrent(); 

    io::print("We are starting our first task\n");
    core_tasks_switch_esp(&dummyEsp, &m_tasks.begin()->m_kernelStack.esp);
    __builtin_unreachable();
  }

  Task& Scheduler::addTask(Task task)
  {
    auto it = m_tasks.insert(m_tasks.end(), std::move(task));
    return *it;
  }

  void Scheduler::removeTask(Task& task)
  {
    // FIXME: We can do better than that
    for(auto it = m_tasks.begin(); it!=m_tasks.end(); ++it)
      if(&(*it) == &task)
      {
        m_tasks.erase(it);
        return;
      }
  }

  void Scheduler::schedule()
  {
    assert(!m_tasks.empty());

    // We can actually convert pointer to iterator directly if we export 
    // a interface from the container to do so
    for(auto it = m_tasks.begin(); it != std::prev(m_tasks.end()); ++it)
      if(Task::current == &(*it))
      {
        switchTask(*std::next(it));
        return;
      }

    switchTask(*m_tasks.begin());
  }

  void Scheduler::switchTask(Task& nextTask) const
  {
    assert(Task::current != nullptr);
    auto& previousTask = *Task::current;
    nextTask.makeCurrent(); 
    core_tasks_switch_esp(&previousTask.m_kernelStack.esp, &nextTask.m_kernelStack.esp);
  }
}
