#include <core/generic/tasks/Scheduler.hpp>
#include <core/generic/memory/Memory.hpp>
#include <core/generic/io/Print.hpp>
#include <core/generic/Panic.hpp>
#include <core/i686/tasks/Task.hpp>
#include <core/i686/syscalls/Syscalls.hpp>

#include <core/generic/Modules.hpp>

#include <assert.h>
#include <optional>

void testMemory()
{
  core::io::print("Testing memory allocation and deallocation...");

  for(size_t j=0; j<500;++j)
  {
    char* memorys[200] = {};
    for(size_t i=0; i<200; ++i)
    {
      memorys[i] = static_cast<char*>(core::memory::malloc(0x1000));
      if(memorys[i] == nullptr)
        break;

      const char* str = "deadbeef";
      for(size_t k=0; k<0x100; ++k)
      {
        static_cast<volatile char*>(memorys[i])[k]=str[k%8];
        assert(static_cast<volatile char*>(memorys[i])[k]==str[k%8]);
      }
    }

    //for(size_t i=0; i<200; ++i)
    //{
    //  if(memorys[i] == nullptr)
    //    break;
    //  core::memory::free(static_cast<void*>(memorys[i]));
    //}
  }

  core::io::print("Done\n");
}

template<typename T>
T unwrap(std::optional<T> t)
{
  if(!t)
    core::panic("Null optional");

  return std::move(*t);
}

void kmain()
{
  // Memory System
  testMemory();

  if(core::modulesCount != 0)
  {
    core::syscalls::installHandler(1, [](int, int, int, int){ core::io::print("Hello from kernel\n"); return 0;});

    for(size_t i=0; i<core::modulesCount; ++i)
    {
      // TODO: Parse elf
      const auto& module = core::modules[i];
      auto& task = core::tasks::scheduler.addTask(unwrap(core::tasks::Task::allocate()));
      task.asUserspaceTask(0x0);

      auto physicalPages = core::memory::Pages::fromAggressive(module.phyaddr, module.length);
      auto virtualPages  = core::memory::Pages::fromAggressive(0             , module.length);

      // TODO: Load Actual from Executable Format
      task.memoryMapping().map(physicalPages, virtualPages, common::memory::Access::ALL, common::memory::Permission::READ_WRITE);
    }

    core::tasks::scheduler.startFirstUserspaceTask(0x0);
  }
}
