#include <i686/tasks/Task.hpp>

#include <generic/memory/Memory.hpp>
#include <i686/memory/MemoryMapping.hpp>

#include <generic/tasks/Scheduler.hpp>

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

  // FIXME: Implement for multiprocessor
  constinit rt::SharedPtr<Task> Task::current;

  void Task::makeCurrent(rt::SharedPtr<Task> task)
  {
    if(current && current->state == State::RUNNING)
      current->state = State::RUNNABLE;

    current = task;
    if(task)
    {
      ASSERT(current->state == State::RUNNABLE);
      current->state = State::RUNNING;

      interrupts::setKernelStack(reinterpret_cast<uintptr_t>(task->kernelStack.ptr), STACK_SIZE);
      syscalls::setKernelStack(reinterpret_cast<uintptr_t>(task->kernelStack.ptr), STACK_SIZE);
      if(task->memoryMapping)
        memory::MemoryMapping::makeCurrent(task->memoryMapping); // Kernel task do not have memory mapping and would use the old one
    }
  }

  void Task::switchTo(rt::SharedPtr<Task> task)
  {
    if(current.get() != nullptr)
    {
      auto previousTask = current;

      makeCurrent(task);
      core_tasks_switch_esp(&previousTask->kernelStack.esp, &task->kernelStack.esp);
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
      core_tasks_switch_esp(&dummyEsp, &task->kernelStack.esp);
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

  rt::SharedPtr<Task> Task::clone()
  {
    auto task = allocate();
    if(!task)
      return nullptr;

    task->fileDescriptors = this->fileDescriptors;

    Registers registers = this->registers;
    registers.eax = 0;
    task->asUserTask(registers);

    if(this->memoryMapping)
    {
      task->memoryMapping = this->memoryMapping->clone();
      if(!task->memoryMapping)
        return nullptr; // Clone failed
    }

    return task;
  }

  Task::Task(pid_t pid, Stack kernelStack)
    : pid(pid), kernelStack(kernelStack) {}

  Task::~Task()
  {
    memory::freePages(kernelStack.ptr, STACK_PAGES_COUNT);
  }

  void Task::kill(status_t status)
  {
    this->state = State::DEAD;
    this->status = status;
  }

  namespace
  {
    template<typename T>
    void push(Stack& stack, const T& value) requires(std::is_trivially_copyable_v<T>)
    {
      stack.esp -= sizeof value;
      *reinterpret_cast<T*>(stack.esp) = value;
    }

    void newKernelTask(void(*kernelTask)())
    {
      rt::logf("Launching new kernel tasks\n");
      kernelTask();
      killCurrent(0);
      schedule();
      ASSERT_UNREACHABLE;
    }

    void newUserTask(Registers& registers)
    {
      rt::logf("Launching new user tasks with eip=0x%lx, esp=0x%lx\n", registers.eip, registers.esp);
      core_tasks_entry(&registers);
      ASSERT_UNREACHABLE;
    }
  }

  Result<void> Task::asKernelTask(void(*kernelTask)())
  {
    push(kernelStack, kernelTask);
    push(kernelStack, POISON); // This can really be any value, we are just simulating eip pushed when executing a call instruction
    push(kernelStack, &newKernelTask);

    return {};
  }

  Result<void> Task::asUserTask(Registers registers)
  {
    this->registers = registers;

    push(kernelStack, &this->registers);
    push(kernelStack, POISON); // This can really be any value, we are just simulating eip pushed when executing a call instruction
    push(kernelStack, &newUserTask);

    ASSERT(!memoryMapping);
    memoryMapping = memory::MemoryMapping::allocate();
    if(!memoryMapping)
      return ErrorCode::OUT_OF_MEMORY;

    return {};
  }

}
