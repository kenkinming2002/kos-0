#include <i686/core/Syscall.hpp>


namespace core
{
  namespace
  {
    [[gnu::naked]] void syscall_handler()
    {
      asm volatile ( R"(
        .intel_syntax noprefix
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
