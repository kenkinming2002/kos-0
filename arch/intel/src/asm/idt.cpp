#include <intel/asm/idt.hpp>

namespace assembly
{
  void lidt(const void* idt)
  {
    asm volatile ( R"(
      .intel_syntax noprefix
        lidt [%[idt]]
      )"
      :
      : [idt]"r"(idt)
      :
    );
  } 
}
