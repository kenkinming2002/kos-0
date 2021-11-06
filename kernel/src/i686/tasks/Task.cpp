#include <i686/tasks/Task.hpp>

#include <generic/Init.hpp>
#include <generic/PerCPU.hpp>
#include <generic/tasks/Scheduler.hpp>

#include <i686/tasks/Switch.hpp>
#include <i686/interrupts/Interrupts.hpp>
#include <i686/syscalls/Syscalls.hpp>
#include <i686/tasks/Entry.hpp>

#include <librt/Global.hpp>

namespace core::tasks
{
  static constexpr uintptr_t POISON = 0xDEADBEEF;

  void Task::switchTo(Task* oldTask, Task& newTask)
  {
    interrupts::setKernelStack(reinterpret_cast<uintptr_t>(newTask.kernelStack.ptr), STACK_SIZE);
    syscalls::setKernelStack(reinterpret_cast<uintptr_t>(newTask.kernelStack.ptr), STACK_SIZE);
    if(newTask.memoryMapping)
      memory::MemoryMapping::makeCurrent(newTask.memoryMapping);

    uintptr_t dummyEsp;
    uintptr_t& oldEsp = oldTask ? oldTask->kernelStack.esp : dummyEsp;
    uintptr_t& newEsp = newTask.kernelStack.esp;
    core_tasks_switch_esp(&oldEsp, &newEsp);
    // Only return if oldEsp is not a dummy
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

    return rt::makeShared<Task>(stack);
  }

  rt::SharedPtr<Task> Task::clone()
  {
    auto task = allocate();
    if(!task)
      return nullptr;

    Registers registers = this->registers;
    registers.eax = 0;
    if(this->memoryMapping)
    {
      task->memoryMapping = this->memoryMapping->clone();
      if(!task->memoryMapping)
        return nullptr; // Clone failed
    }
    if(this->fileDescriptors)
    {
      task->fileDescriptors = this->fileDescriptors;
      if(!task->fileDescriptors)
        return nullptr; // Clone failed
    }
    task->asUserTask(registers);
    return task;
  }

  Task::Task(Stack kernelStack)
    : kernelStack(kernelStack) {}

  Task::~Task()
  {
    memory::freePages(kernelStack.ptr, STACK_PAGES_COUNT);
  }
  namespace
  {
    template<typename T>
    void push(Stack& stack, const T& value) requires(std::is_trivially_copyable_v<T>)
    {
      stack.esp -= sizeof value;
      *reinterpret_cast<T*>(stack.esp) = value;
    }

    void newKernelTask(void(*kernelTask)(void*), void* data)
    {
      kernelTask(data);
      killCurrent(0);
      schedule();
      ASSERT_UNREACHABLE;
    }

    void newUserTask(Registers& registers)
    {
      core_tasks_entry(&registers);
      ASSERT_UNREACHABLE;
    }
  }

  Result<void> Task::asKernelTask(void(*kernelTask)(void*), void* data)
  {
    push(kernelStack, data);
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

    if(!memoryMapping)
    {
      memoryMapping = memory::MemoryMapping::allocate();
      if(!memoryMapping)
        return ErrorCode::OUT_OF_MEMORY;
    }

    if(!fileDescriptors)
    {
      fileDescriptors = rt::makeShared<vfs::FileDescriptors>();
      if(!fileDescriptors)
        return ErrorCode::OUT_OF_MEMORY;
    }

    return {};
  }

}
