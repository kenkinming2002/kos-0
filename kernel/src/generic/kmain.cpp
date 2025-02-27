#include <generic/vfs/VFS.hpp>

#include <generic/log/Log.hpp>
#include <generic/Init.hpp>
#include <generic/PerCPU.hpp>
#include <generic/tasks/Tasks.hpp>
#include <generic/tasks/Elf.hpp>
#include <generic/tasks/Scheduler.hpp>
#include <generic/devices/Framebuffer.hpp>
#include <generic/memory/Memory.hpp>
#include <generic/timers/Timer.hpp>
#include <generic/BootInformation.hpp>

#include <i686/syscalls/Access.hpp>
#include <i686/internals/Internals.hpp>
#include <i686/interrupts/Interrupts.hpp>

#include <i686/syscalls/Syscalls.hpp>

#include <x86/LocalAPIC.hpp>
#include <x86/SMP.hpp>
#include <x86/acpi/ACPI.hpp>
#include <x86/interrupts/PIC.hpp>

#include <librt/StringRef.hpp>
#include <librt/Log.hpp>
#include <librt/Assert.hpp>

static core::Result<result_t> _sys_test()
{
  rt::logf("Hello from kernel\n");
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
  rt::logf("User Interrupt\n");
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

[[noreturn]] static void initialize(BootInformation* bootInformation, unsigned apicid)
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

  // Platform specific initialization
  //
  // Boot information in form of ACPI
  // and Local APIC - VERY IMPORTANT
  core::acpi::initialize();
  core::LocalAPIC::initialize();

  core::interrupts::initializePIC();

  core::vfs::initialize();
  core::timers::initialize();
  core::tasks::initialize();

  // Userspace specific initialization
  core::memory::initializeVirtual();

  core::syscalls::installHandler(SYS_TEST, &sys_test);
  core::syscalls::installHandler(SYS_LOG,  &sys_log);
  core::interrupts::installHandler(0x80, &irq_test , core::PrivilegeLevel::RING3, true);


  // Actual userspace init task
  {
    auto root = core::vfs::root();
    auto init = core::vfs::openAt(root, "init");
    if(!init)
      rt::panic("init not found\n");

    auto task = core::tasks::Task::allocate();
    if(!task)
      rt::panic("Failed to create task\n");

    core::tasks::loadElf(task, *init);
    addTask(task);
  }

  core::foreachCPUInitCall([](){
    core::tasks::schedule();
    core::tasks::killCurrent(0);
    core::tasks::onResume();
    ASSERT_UNREACHABLE;
  });
  __builtin_unreachable();
}

extern "C" void kmain(BootInformation* bootInformation, unsigned apicid)
{
  core::initializeSMP(bootInformation, apicid);
  initialize(bootInformation, apicid);
}
