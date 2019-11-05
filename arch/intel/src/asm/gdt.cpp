#include <intel/asm/gdt.hpp>

namespace assembly
{
  void lgdt(const void* gdt)
  {
    asm volatile ( R"(
      .intel_syntax noprefix
        lgdt [%[gdt]]
      .att_syntax prefix
      )"
      :
      : [gdt]"r"(gdt)
      :
    );
  }

  void gdt_flush()
  {
    asm volatile ( R"(
      .intel_syntax noprefix
        mov ax, 0x10
        mov ds, ax
        mov ss, ax
        mov es, ax
        jmp 0x08:flush_cs
      flush_cs:
      .att_syntax prefix
      )"
      :
      :
      : "ax"
    );
  }
}
