#pragma once

#include <sys/Types.hpp>

namespace core::tasks
{
  struct [[gnu::packed]] Registers
  {
    uword_t eax;
    uword_t ebx;
    uword_t ecx;
    uword_t edx;
    uword_t esi;
    uword_t edi;
    uword_t ebp;
    uword_t esp;
    uword_t eip;
  };
}
