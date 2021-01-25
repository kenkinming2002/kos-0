#include "common/i686/memory/Paging.hpp"
#include "generic/tasks/Elf.hpp"
#include "i686/memory/MemoryMapping.hpp"
#include <generic/tasks/Scheduler.hpp>
#include <generic/memory/Memory.hpp>
#include <common/generic/io/Print.hpp>
#include <generic/Panic.hpp>
#include <i686/tasks/Task.hpp>
#include <i686/syscalls/Syscalls.hpp>

#include <generic/Modules.hpp>

#include <assert.h>
#include <optional>

void testMemory()
{
  core::io::print("Testing memory allocation and deallocation...");

  for(size_t j=0; j<512;++j)
  {
    core::io::printf("Iteration %lu\n", j);
    char* memorys[200] = {};

    for(size_t i=0; i<200; ++i)
    {
      memorys[i] = static_cast<char*>(core::memory::malloc(0x1000));

      /* Note: Don't be stupid like me and try to use break to break out of
       * nested loop and be puzzled as to why the loop does not end */
      if(memorys[i] == nullptr)
        goto end;  

      const char* str = "deadbeef";
      for(size_t k=0; k<0x100; ++k)
      {
        static_cast<volatile char*>(memorys[i])[k]=str[k%8];
        assert(static_cast<volatile char*>(memorys[i])[k]==str[k%8]);
      }
    }

    for(size_t i=0; i<200; ++i)
    {
      if(memorys[i] == nullptr)
        break;
      core::memory::free(static_cast<void*>(memorys[i]));
    }
  }
end:

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

    for(size_t i=1; i<core::modulesCount; ++i)
    {
      // TODO: Parse elf
      const auto& module = core::modules[i];

      auto physicalPages = core::memory::Pages::fromAggressive(module.phyaddr, module.length);
      auto virtualPages  = core::memory::allocVirtualPages(physicalPages.count);
      if(!virtualPages)
        continue;

      core::memory::MemoryMapping::current->map(*virtualPages, common::memory::Access::SUPERVISOR_ONLY, common::memory::Permission::READ_ONLY, physicalPages);
      auto task = core::tasks::loadElf(reinterpret_cast<char*>(virtualPages->address()), virtualPages->length());
      if(!task)
        core::panic("Failed to load elf\n");

      core::tasks::scheduler.addTask(std::move(*task));

      core::memory::MemoryMapping::current->unmap(*virtualPages);
      core::memory::freeVirtualPages(*virtualPages);
    }
    core::io::print("Done\n");
    core::tasks::scheduler.startFirstUserspaceTask();
  }
}
