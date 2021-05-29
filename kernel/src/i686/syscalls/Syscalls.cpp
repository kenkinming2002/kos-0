#include <i686/syscalls/Syscalls.hpp>

#include <i686/syscalls/Entry.hpp>

#include <x86/assembly/msr.hpp>

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

  namespace
  {
    bool verifyUserBuffer(const char* buffer, size_t length)
    {
      /* We only do prelimary check, and leave the rest to be handled via page
       * fault */
      return reinterpret_cast<uintptr_t>(buffer) < std::numeric_limits<uintptr_t>::max()-length &&
             reinterpret_cast<uintptr_t>(buffer)+length<0xC0000000;
    }

    bool copyFromUser(const char* userBuffer, char* kernelBuffer, size_t length)
    {
      if(!verifyUserBuffer(userBuffer, length))
        return false;

      /* TODO: Gracefully handle page fault. In linux, this is handled by the
       * __might_fault() function, which save the address after the instruction
       * that might fault in a lookup table and allow the page fault handler to
       * resume execution. */
      rt::copy(userBuffer, userBuffer+length, kernelBuffer);
      return true;
    }
  }

  extern "C" int core_syscalls_dispatch(int syscallNumber, int a1, int a2, int a3, int espUser)
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

    // Retrieve the arguments
    int stackArgs[3] = {};
    if(!copyFromUser(reinterpret_cast<const char*>(espUser), reinterpret_cast<char*>(stackArgs), sizeof stackArgs))
      return -1; // TODO: Error code

    return handlers[syscallNumber](syscallNumber, a1, a2, a3, stackArgs[0], stackArgs[1], stackArgs[2]);
  }
}
