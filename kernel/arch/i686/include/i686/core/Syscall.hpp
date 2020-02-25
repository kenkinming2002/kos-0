#pragma once

#include <stdint.h>
#include <stddef.h>

namespace core
{
  struct Command
  {
  public:
    enum class OpCode : uint8_t
    {
      END   = 0,
      OPEN  = 1,
      CLOSE = 2,
      READ  = 3,
      WRITE = 4
    };

    struct Operand
    {
      enum class Type : uint8_t
      {
        IMMEDIATE = 0,
        INDIRECT  = 1
      } type;

      struct Immediate { size_t value; };
      struct Indirect { size_t index; };
      union { Immediate immediate; Indirect indirect; };
    };

  public:
    OpCode opCode;
    Operand operand[];
  };

  struct SyscallResult
  {
    int retval;
    size_t operandCount;
  };
  using SyscallHandler = SyscallResult(*)(const Command::Operand*); // Rare case for east const

  void init_syscall();

  void register_syscall_handler(Command::OpCode opCode, SyscallHandler syscallHandler);
  void set_syscall_esp(uintptr_t esp);
}
