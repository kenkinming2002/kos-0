#include <i686/tasks/Task.hpp>

#include <generic/Init.hpp>
#include <generic/Panic.hpp>
#include <generic/memory/Memory.hpp>
#include <i686/interrupts/Interrupts.hpp>
#include <i686/syscalls/Syscalls.hpp>
#include <i686/tasks/Entry.hpp>

#include <cassert>

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

  constinit Task* Task::current = nullptr; // FIXME: Implement for multiprocessor

  std::optional<Task> Task::allocate()
  {
    auto memoryMapping = memory::MemoryMapping::allocate();
    if(!memoryMapping)
      return std::nullopt;

    auto kernelStackPage = memory::allocMappedPages(1);
    if(!kernelStackPage)
      return std::nullopt;

    auto stack = Stack{
      .ptr = kernelStackPage->address(), 
      .size = kernelStackPage->length(),
      .esp = kernelStackPage->address() + kernelStackPage->length()
    };

    return Task(stack, std::move(*memoryMapping));
  }

  Task::Task(Stack kernelStack, memory::MemoryMapping memoryMapping)
    : m_kernelStack(kernelStack), m_memoryMapping(std::move(memoryMapping)) {}

  Task::~Task()
  {
    if(m_kernelStack.ptr == 0 || m_kernelStack.size == 0)
      return;

    auto kernelStackPage = memory::Pages::from(m_kernelStack.ptr, m_kernelStack.size);
    memory::freeMappedPages(kernelStackPage);
  }

  Task::Task(Task&& other)
    : m_kernelStack{std::exchange(other.m_kernelStack, Stack{.ptr=0,.size=0,.esp=0})},
      m_memoryMapping(std::move(other.m_memoryMapping)) {}

  Task& Task::operator=(Task&& other)
  {
    m_memoryMapping = std::move(other.m_memoryMapping);
    std::swap(m_kernelStack, other.m_kernelStack);
    return *this;
  }

  void Task::makeCurrent()
  {
    current = this;
    interrupts::setKernelStack(m_kernelStack.ptr, m_kernelStack.size);
    syscalls::setKernelStack(m_kernelStack.ptr, m_kernelStack.size);
    m_memoryMapping.makeCurrent();
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
