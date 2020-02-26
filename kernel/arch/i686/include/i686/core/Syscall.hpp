#pragma once

#include <stdint.h>
#include <stddef.h>

namespace core
{
  struct Registers
  {
    uint32_t edi; //< Argument 4
    uint32_t esi; //< Argument 3
    uint32_t ebp; //< Argument 2
    uint32_t esp; //< Kernel ESP
    uint32_t ebx; //< Argument 1
    uint32_t edx; //< User Program ESP
    uint32_t ecx; //< User Program EIP
    uint32_t eax; //< Syscall Number
  };

  using SyscallHandler = int(*)(const Registers); // Rare case for east const

  void init_syscall();

  void register_syscall_handler(uint8_t syscallNumber, SyscallHandler syscallHandler);
  void set_syscall_esp(uintptr_t esp);
}
