#include <generic/log/Log.hpp>
#include <generic/tasks/Elf.hpp>
#include <generic/devices/Framebuffer.hpp>
#include <generic/tasks/Scheduler.hpp>
#include <generic/memory/Memory.hpp>
#include <generic/memory/Physical.hpp>
#include <generic/memory/Virtual.hpp>
#include <generic/BootInformation.hpp>

#include <i686/internals/Internals.hpp>
#include <i686/interrupts/Interrupts.hpp>
#include <i686/memory/MemoryMapping.hpp>
#include <i686/tasks/Task.hpp>
#include <i686/syscalls/Syscalls.hpp>

#include <librt/Optional.hpp>
#include <librt/Log.hpp>
#include <librt/Panic.hpp>
#include <librt/Assert.hpp>

static void testMemory()
{
  rt::log("Testing memory allocation and deallocation...\n");

  for(size_t j=0; j<128;++j)
  {
    rt::logf("Iteration %lu\n", j);
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
        ASSERT(static_cast<volatile char*>(memorys[i])[k]==str[k%8]);
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

  rt::log("Done\n");
}

static void kmainInitialize(BootInformation* bootInformation)
{
  ::bootInformation = bootInformation;

  core::log::initialize();
  core::internals::initialize();
  core::interrupts::initialize();
  core::memory::initialize();
  core::syscalls::initialize();
  core::tasks::initializeScheduler();
}

[[noreturn]] static void kmainLoadAndRunUserspaceTask()
{
  if(bootInformation->moduleEntriesCount == 0)
    rt::panic("No modules to run");

  for(size_t i=1; i<bootInformation->moduleEntriesCount; ++i)
  {
    // TODO: Parse elf
    const auto& module = bootInformation->moduleEntries[i];
    auto pages = core::memory::mapPages(core::memory::Pages::fromAggressive(module.addr, module.len));
    if(!pages)
      continue;

    auto task = core::tasks::addTask();
    if(!task)
      rt::panic("Failed to create task\n");
    if(core::tasks::loadElf(*task, reinterpret_cast<char*>(pages->address()), pages->length()) != 0)
      rt::panic("Failed to load ELF\n");

    core::memory::freeMappedPages(*pages);
  }
  rt::log("Done\n");
  core::tasks::scheduleInitial();
}

extern "C" void kmain(BootInformation* bootInformation)
{
  kmainInitialize(bootInformation);
  testMemory();

  core::syscalls::installHandler(1, [](int, int, int, int){ rt::log("Hello from kernel\n"); return 0;});
  core::interrupts::installHandler(0x80, [](uint8_t, uint32_t, uintptr_t) { rt::log("User Interrupt\n"); }, core::PrivilegeLevel::RING3, true);

  kmainLoadAndRunUserspaceTask();
}
