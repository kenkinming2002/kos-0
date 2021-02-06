#include <i686/syscalls/Syscalls.hpp>

#include <i686/syscalls/Entry.hpp>

#include <x86/assembly/msr.hpp>

#include <generic/Panic.hpp>
#include <common/generic/io/Print.hpp>

namespace core::syscalls
{
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
      panic("Attempting to install mutiple syscall handlers for same syscall number\n"
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

  extern "C" int core_syscalls_dispatch(int syscallNumber, int a1, int a2, int a3)
  {
    if(syscallNumber<0 || syscallNumber>=MAX_SYSCALL_COUNT)
    {
      core::io::print("Syscall numbers out of range\n");
      return -1;
    }

    if(!handlers[syscallNumber])
    {
      core::io::print("Unknown Syscalls\n");
      return -1; // TODO: Error code
    }

    return handlers[syscallNumber](syscallNumber, a1, a2, a3);
  }
}
