#pragma once

#include <stdint.h>
#include <stddef.h>

namespace core
{
  struct State
  {
    union { uint32_t edi; uint32_t a4; }; //< Argument 4
    union { uint32_t esi; uint32_t a3; }; //< Argument 3
    union { uint32_t ebp; uint32_t a2; }; //< Argument 3
    uint32_t esp; 
    union { uint32_t ebx; uint32_t a1; }; //< Argument 1
    uint32_t edx; 
    uint32_t ecx;
    union { uint32_t eax; uint32_t syscall_number; }; //< Argument 1

    uint32_t eip;
  } __attribute__((packed));

  using SyscallHandler = int(*)(const State); // Rare case for east const

  void init_syscall();

  void register_syscall_handler(uint8_t syscallNumber, SyscallHandler syscallHandler);
  void set_syscall_esp(uintptr_t esp);
}
