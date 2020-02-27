#include <i686/core/Syscall.hpp>

#include <string.h>

namespace core
{
  SyscallHandler syscallTable[256] = {0};

  namespace
  {
    struct Registers
    {
      uint32_t edi; //< Argument 4
      uint32_t esi; //< Argument 3
      uint32_t ebp; //< Argument 2
      uint32_t esp; //< Kernel ESP
      uint32_t ebx; //< Argument 1
      uint32_t edx; //< User Program EIP
      uint32_t ecx; //< User Program ESP
      uint32_t eax; //< Syscall Number

      State toState() const
      {
        State state; 
        memcpy(&state, this, sizeof(Registers));

        state.esp = this->ecx;
        state.eip = this->edx;

        return state;
      }
    } __attribute__((packed));

    extern "C" int _syscall_handler(const Registers registers)
    {
      // NOTE: We have to check if command is valid
      if(auto syscallHandler = syscallTable[registers.eax & 0xFF])
        return syscallHandler(registers.toState());
      else
        return -1;
    }

    [[gnu::naked]] void syscall_handler()
    {
      // NOTE: Maybe there is no need to restore the kernel stack?
      //
      // Turn out sysenter disable interrupt so we have to enable it at the end.
      // This is poorly documented and not mentioned in Intel Programmer's
      // Manual System Programming Guide(i.e Volume 3). For infomation, consult
      // the Instruction Set Reference(i.e. Volume 1) from Intel Programmer's
      // Manual.
      asm volatile ( R"(
        .intel_syntax noprefix
          pushad
          mov ax, 0x10
          mov ds, ax
          mov es, ax
          mov fs, ax
          mov gs, ax

          call _syscall_handler

          mov ax, 0x23
          mov ds, ax
          mov es, ax
          mov fs, ax
          mov gs, ax
          popad

          sti
          sysexit
        .att_syntax prefix
        )"
        :
        : 
        :
      );
    }
  }

  void init_syscall()
  {
    // Initialize stack segment selector and value of eip for sysenter
    asm volatile ( R"(
      .intel_syntax noprefix
        mov ecx, 0x174
        mov eax, 0x8
        mov edx, 0x0
        wrmsr

        mov ecx, 0x176
        mov eax, %[syscall_handler]
        mov edx, 0x0
        wrmsr

      .att_syntax prefix
      )"
      :
      : [syscall_handler]"r"(reinterpret_cast<uintptr_t>(&syscall_handler))
      : "ecx", "eax", "edx"
    );
  }

  void register_syscall_handler(uint8_t syscallNumber, SyscallHandler syscallHandler)
  {
    auto& syscallHandlerEntry = syscallTable[syscallNumber];
    if(!syscallHandlerEntry)
      syscallHandlerEntry = syscallHandler;
  }

  void set_syscall_esp(uintptr_t esp)
  {
    // Initialize esp for sysenter
    asm volatile ( R"(
      .intel_syntax noprefix
        mov ecx, 0x175
        mov eax, %[esp]
        mov edx, 0x0
        wrmsr
      .att_syntax prefix
      )"
      :
      : [esp]"r"(esp)
      : "ecx", "eax", "edx"
    );
  }
}
