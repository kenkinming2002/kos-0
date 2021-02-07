#include <generic/tasks/Elf.hpp>
#include <generic/devices/Framebuffer.hpp>
#include <generic/tasks/Scheduler.hpp>
#include <generic/memory/Memory.hpp>
#include <generic/BootInformation.hpp>
#include <generic/Panic.hpp>

#include <i686/internals/Internals.hpp>
#include <i686/interrupts/Interrupts.hpp>
#include <i686/memory/MemoryMapping.hpp>
#include <i686/tasks/Task.hpp>
#include <i686/syscalls/Syscalls.hpp>

#include <common/generic/io/Print.hpp>

#include <assert.h>
#include <optional>

static void testMemory()
{
  core::io::print("Testing memory allocation and deallocation...");

  for(size_t j=0; j<128;++j)
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

static void kmainInitialize(BootInformation* bootInformation)
{
  ::bootInformation = bootInformation;

  core::devices::Framebuffer::initialize();
  core::internals::initialize();
  core::interrupts::initialize();
  core::memory::MemoryMapping::initialize();
  core::memory::initialize();
  core::tasks::Scheduler::initialize();

}

[[noreturn]] static void kmainLoadAndRunUserspaceTask()
{
  if(bootInformation->moduleEntriesCount == 0)
    core::panic("No modules to run");

  for(size_t i=1; i<bootInformation->moduleEntriesCount; ++i)
  {
    // TODO: Parse elf
    const auto& module = bootInformation->moduleEntries[i];

    auto physicalPages = core::memory::Pages::fromAggressive(module.addr, module.len);
    auto virtualPages  = core::memory::allocVirtualPages(physicalPages.count);
    if(!virtualPages)
      continue;

    core::memory::MemoryMapping::current().map(*virtualPages, common::memory::Access::SUPERVISOR_ONLY, common::memory::Permission::READ_ONLY, physicalPages);
    auto task = core::tasks::Scheduler::instance().addTask();
    if(!task)
      core::panic("Failed to create task\n");
    if(core::tasks::loadElf(*task, reinterpret_cast<char*>(virtualPages->address()), virtualPages->length()) != 0)
      core::panic("Failed to load ELF\n");

    core::memory::MemoryMapping::current().unmap(*virtualPages);
    core::memory::freeVirtualPages(*virtualPages);
  }
  core::io::print("Done\n");
  core::tasks::Scheduler::instance().startFirstUserspaceTask();
}

extern "C" void kmain(BootInformation* bootInformation)
{
  kmainInitialize(bootInformation);
  testMemory();

  core::syscalls::installHandler(1, [](int, int, int, int){ core::io::print("Hello from kernel\n"); return 0;});
  core::interrupts::installHandler(0x80, [](uint8_t, uint32_t, uintptr_t) { core::io::print("User Interrupt\n"); }, core::PrivilegeLevel::RING3, true);

  kmainLoadAndRunUserspaceTask();
}
