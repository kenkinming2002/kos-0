#include <i686/core/Syscall.hpp>


namespace core
{
  SyscallHandler syscallTable[256] = {0};

  namespace
  {
    extern "C" [[gnu::no_caller_saved_registers]] int _syscall_handler(const Command* command)
    {
      // NOTE: We have to check if command is valid
      for(;;)
      {
        if(reinterpret_cast<uintptr_t>(&command->operand)>0xC0000000)
          return -1; // Access Violation

        if(command->opCode == Command::OpCode::END)
          return 0; // All Commands Successfully Executed

        if(auto syscallHandler = syscallTable[static_cast<uint8_t>(command->opCode)])
        {
          const Command::Operand* operand = command->operand;
          auto syscallResult = syscallHandler(operand);
          if(syscallResult.retval != 0)
            return syscallResult.retval;
          command = reinterpret_cast<const Command*>(operand + syscallResult.operandCount); // End of operand is start of next command
        }
        else
          return -1;
      }

      return 0;
    }

    [[gnu::naked]] void syscall_handler()
    {
      // NOTE: Maybe there is no need to restore the kernel stack?
      asm volatile ( R"(
        .intel_syntax noprefix

          push eax
          call _syscall_handler
          add esp, 4

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

  void register_syscall_handler(Command::OpCode opCode, SyscallHandler syscallHandler)
  {
    auto& syscallHandlerEntry = syscallTable[static_cast<uint8_t>(opCode)];
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
