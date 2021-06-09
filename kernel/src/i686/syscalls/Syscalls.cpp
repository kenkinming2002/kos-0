#include <i686/syscalls/Syscalls.hpp>

#include <i686/syscalls/Entry.hpp>
#include <i686/syscalls/Access.hpp>

#include <x86/assembly/msr.hpp>

#include <librt/Optional.hpp>
#include <librt/Algorithm.hpp>
#include <librt/Panic.hpp>
#include <librt/Log.hpp>

namespace core::syscalls
{
  inline word_t _sys_test(const char*, const char*, void*, size_t, size_t, intptr_t)
  {
    return -1;
  }
  WRAP_SYSCALL6(sys_test, _sys_test)

  constexpr uint32_t IA32_SYSENTER_CS = 0x174;
  constexpr uint32_t IA32_SYSENTER_ESP = 0x175;
  constexpr uint32_t IA32_SYSENTER_EIP = 0x176;

  void initialize()
  {
    assembly::wrmsr(IA32_SYSENTER_CS, 0x8);
    assembly::wrmsr(IA32_SYSENTER_EIP, reinterpret_cast<uintptr_t>(&core_syscalls_entry));
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

  using result_t = word_t;
  extern "C" result_t core_syscalls_dispatch(uword_t syscallNumber, uword_t a1, uword_t a2, uword_t a3, uword_t espUser)
  {
    if(syscallNumber<0 || syscallNumber>=MAX_SYSCALL_COUNT)
    {
      rt::log("Syscall numbers out of range\n");
      return -1;
    }

    if(!handlers[syscallNumber])
    {
      rt::log("Unknown Syscalls\n");
      return -1; // TODO: Error code
    }

    // Retrieve stack arguments
    uword_t a4, a5, a6;
    {
      int stackArgs[3] = {};
      auto buffer = InputUserBuffer(reinterpret_cast<const char*>(espUser), sizeof stackArgs);
      if(auto result = buffer.read(reinterpret_cast<char*>(stackArgs), sizeof stackArgs); !result)
        return -static_cast<result_t>(result.error());

      a4 = stackArgs[0];
      a5 = stackArgs[1];
      a6 = stackArgs[2];
    }

    return handlers[syscallNumber](a1, a2, a3, a4, a5, a6);
  }
}
