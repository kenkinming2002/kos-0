#include <generic/vfs/VFS.hpp>

#include <generic/log/Log.hpp>
#include <generic/Init.hpp>
#include <generic/PerCPU.hpp>
#include <generic/tasks/Tasks.hpp>
#include <generic/tasks/Scheduler.hpp>
#include <generic/devices/Framebuffer.hpp>
#include <generic/memory/Memory.hpp>
#include <generic/BootInformation.hpp>

#include <i686/syscalls/Access.hpp>
#include <i686/internals/Internals.hpp>
#include <i686/interrupts/Interrupts.hpp>

#include <i686/syscalls/Syscalls.hpp>

#include <x86/acpi/ACPI.hpp>
#include <x86/interrupts/PIC.hpp>

#include <librt/StringRef.hpp>
#include <librt/Log.hpp>
#include <librt/Assert.hpp>

static core::Result<result_t> _sys_test()
{
  rt::log("Hello from kernel\n");
  return 0;
}
WRAP_SYSCALL0(sys_test, _sys_test)

static core::Result<result_t> _sys_log(const char* msg, size_t length)
{
  char buf[length+1];
  auto result = core::syscalls::InputUserBuffer(msg, length).read(buf, length);
  UNWRAP(result);
  buf[length] = '\0';

  rt::logf("%s", buf);
  return length;
}
WRAP_SYSCALL2(sys_log, _sys_log)

static void irq_test(irq_t, uword_t, uintptr_t)
{
  rt::log("User Interrupt\n");
}

static std::atomic<unsigned> count = 0;
static void test() { ++count; }
void testInitCall()
{
  static constexpr size_t COUNT = 10;
  for(size_t i=0; i<COUNT; ++i)
    core::foreachCPUInitCall(&test);

  ASSERT(count == COUNT * bootInformation->coresCount);
}

[[noreturn]] static void runBSP(BootInformation* bootInformation, unsigned apicid)
{
  ::bootInformation = bootInformation;

  core::log::initialize();
  rt::logf("Booting with %u cores\n", bootInformation->coresCount);
  rt::logf("Selected processor with apicid %u as BSP\n", apicid);

  // Early initialization, this service are needed for further initialization
  core::initializePerCPU();
  core::memory::initialize();
  core::internals::initialize();
  core::interrupts::initialize();
  core::syscalls::initialize();

  // Boot information in form of ACPI, also very important
  core::acpi::initialize();
  core::interrupts::initializePIC();

  core::vfs::initialize();
  core::tasks::initialize();

  core::syscalls::installHandler(SYS_TEST, &sys_test);
  core::syscalls::installHandler(SYS_LOG,  &sys_log);
  core::interrupts::installHandler(0x80, &irq_test , core::PrivilegeLevel::RING3, true);

  core::foreachCPUInitCall([](){
    core::tasks::schedule();
    __builtin_unreachable();
  });
  __builtin_unreachable();
}

[[noreturn]] static void runAP()
{
  core::foreachCPUInitHandleLoop();
}

namespace
{
  std::atomic<unsigned> bsp;
  std::atomic<unsigned> initializedCoresCount;
}

extern "C" void kmain(BootInformation* bootInformation, unsigned apicid)
{
  ++initializedCoresCount;
  while(initializedCoresCount != bootInformation->coresCount) asm volatile("pause");

  //if(bsp.exchange(1) == 0)
  if(apicid == 0)
    runBSP(bootInformation, apicid); // This is not necessarily BSP
  else
    runAP();
}
