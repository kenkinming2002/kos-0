#pragma once

#include <stdint.h>
#include <stddef.h>

namespace core
{
  struct State
  {
    union { uint32_t edi; uint32_t a4; int32_t a4_s; }; //< Argument 4
    union { uint32_t esi; uint32_t a3; int32_t a3_s; }; //< Argument 3
    union { uint32_t ebp; uint32_t a2; int32_t a2_s; }; //< Argument 2
    uint32_t esp; 
    union { uint32_t ebx; uint32_t a1; int32_t a1_s; }; //< Argument 1
    uint32_t edx; 
    uint32_t ecx;
    union { uint32_t eax; uint32_t syscall_number; };

    uint32_t eip;
  } __attribute__((packed));

  using SyscallHandler = int(*)(const State); // Rare case for east const

  void init_syscall();

  void register_syscall_handler(uint8_t syscallNumber, SyscallHandler syscallHandler);
  void set_syscall_esp(uintptr_t esp);

  enum Syscall
  {
    SYSCALL_YIELD,

    SYSCALL_SERVICE_CREATE,
    SYSCALL_SERVICE_DESTROY,
    SYSCALL_SERVICE_LOCATE,

    SYSCALL_MESSAGE_SEND,
    SYSCALL_MESSAGE_RECV,
  };
}
