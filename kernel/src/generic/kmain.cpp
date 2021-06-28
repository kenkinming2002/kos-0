#include "generic/Error.hpp"
#include <generic/vfs/Mountable.hpp>
#include <generic/vfs/Path.hpp>
#include <generic/vfs/Inode.hpp>
#include <generic/vfs/Vnode.hpp>
#include <generic/vfs/VFS.hpp>

#include <generic/log/Log.hpp>

#include <generic/tasks/Elf.hpp>
#include <generic/tasks/Scheduler.hpp>

#include <generic/devices/Framebuffer.hpp>

#include <generic/memory/Memory.hpp>

#include <generic/BootInformation.hpp>

#include <i686/syscalls/Access.hpp>
#include <i686/internals/Internals.hpp>
#include <i686/interrupts/Interrupts.hpp>
#include <i686/memory/MemoryMapping.hpp>
#include <i686/tasks/Task.hpp>
#include <i686/syscalls/Syscalls.hpp>

#include <librt/Strings.hpp>
#include <librt/StringRef.hpp>
#include <librt/UniquePtr.hpp>
#include <librt/Optional.hpp>
#include <librt/Log.hpp>
#include <librt/Panic.hpp>
#include <librt/Assert.hpp>
#include <librt/containers/Map.hpp>
#include <librt/String.hpp>

#include <type_traits>

static void kmainInitialize(BootInformation* bootInformation)
{
  ::bootInformation = bootInformation;

  core::log::initialize();
  core::internals::initialize();
  core::interrupts::initialize();
  core::memory::initialize();
  core::syscalls::initialize();
  core::tasks::initializeScheduler();
  core::vfs::initialize();
}

[[noreturn]] static void kmainLoadAndRunUserspaceTask()
{
  if(bootInformation->moduleEntriesCount == 0)
    rt::panic("No modules to run");

  // We no longer need boot modules since the initial executable should be found
  // in initrd
  auto root = core::vfs::root();
  auto init = core::vfs::openAt(root, "init");
  if(!init)
    rt::panic("init not found\n");

  auto task = core::tasks::addTask();
  if(!task)
    rt::panic("Failed to create task\n");

  core::tasks::loadElf(task, *init);
  core::tasks::scheduleInitial();
}

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

extern "C" void kmain(BootInformation* bootInformation)
{
  kmainInitialize(bootInformation);

  core::syscalls::installHandler(SYS_TEST, &sys_test);
  core::syscalls::installHandler(SYS_LOG,  &sys_log);
  core::interrupts::installHandler(0x80, [](uint8_t, uint32_t, uintptr_t) { rt::log("User Interrupt\n"); }, core::PrivilegeLevel::RING3, true);

  kmainLoadAndRunUserspaceTask();
}
