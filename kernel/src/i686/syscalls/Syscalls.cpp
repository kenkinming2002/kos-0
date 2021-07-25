#include <i686/syscalls/Syscalls.hpp>

#include <i686/syscalls/Entry.hpp>
#include <i686/syscalls/Access.hpp>

#include <x86/assembly/msr.hpp>

#include <generic/Init.hpp>
#include <generic/tasks/Scheduler.hpp>
#include <i686/tasks/Registers.hpp>

#include <librt/Optional.hpp>
#include <librt/Algorithm.hpp>
#include <librt/Panic.hpp>
#include <librt/Log.hpp>

namespace core::syscalls
{
  constexpr uint32_t IA32_SYSENTER_CS = 0x174;
  constexpr uint32_t IA32_SYSENTER_ESP = 0x175;
  constexpr uint32_t IA32_SYSENTER_EIP = 0x176;

  void initialize()
  {
    foreachCPUInitCall([]() {
      assembly::wrmsr(IA32_SYSENTER_CS, 0x8);
      assembly::wrmsr(IA32_SYSENTER_EIP, reinterpret_cast<uintptr_t>(&core_syscalls_entry));
    });
  }

  void setKernelStack(uintptr_t ptr, size_t size) { assembly::wrmsr(IA32_SYSENTER_ESP, ptr+size); }

  constinit static Handler handlers[MAX_SYSCALL_COUNT]; // TODO: Record additional information for debugging

  void installHandler(int syscallNumber, Handler handler)
  {
    if(handlers[syscallNumber])
      rt::panic("Attempting to install mutiple syscall handlers for same syscall number\n"
            "  syscall number: %i\n"
            "  old addr: 0x%lx\n"
            "  new addr: 0x%lx\n", syscallNumber,
            reinterpret_cast<uintptr_t>(handlers[syscallNumber]),
            reinterpret_cast<uintptr_t>(handler));

    handlers[syscallNumber] = handler;
  }

  void uninstallHandler(int syscallNumber)
  {
    handlers[syscallNumber] = nullptr;
  }

  Result<result_t> syscallsDispatch(tasks::Registers registers)
  {
    auto i = registers.eax;
    if(i>=MAX_SYSCALL_COUNT || !handlers[i])
      return ErrorCode::INVALID;

    Result<result_t> result = 0;
    {
      /* This is analoguous to BKL in linux, remove it ASAP */
      constinit static rt::SpinLock lock;
      rt::LockGuard guard(lock);

      uword_t args[6];
      args[0] = registers.ebx;
      args[1] = registers.esi;
      args[2] = registers.edi;

      auto length = 3 * sizeof args[0];
      auto buffer = InputUserBuffer(reinterpret_cast<const char*>(registers.esp), length);
      if(auto result = buffer.read(reinterpret_cast<char*>(&args[3]), length); !result)
        return result.error();

      result = handlers[i](args[0], args[1], args[2], args[3], args[4], args[5]);
    }
    tasks::onResume();
    return result;
  }

  inline int makeError(ErrorCode errorCode) { return -static_cast<int>(errorCode); }

  // The registers arg could be modified if necessary
  extern "C" result_t core_syscalls_dispatch(tasks::Registers& registers)
  {
    tasks::Task::current()->registers = registers;
    auto result = syscallsDispatch(registers);
    registers = tasks::Task::current()->registers;
    return result ? *result : makeError(result.error());
  }
}
