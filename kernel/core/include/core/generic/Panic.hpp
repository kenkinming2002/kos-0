#pragma once

#include <core/generic/io/Print.hpp>

namespace core
{
  struct StackFrame
  {
    StackFrame* previous;
    uintptr_t eip;
  };

  inline void traceStackFrame()
  {
    io::print("StackTrace\n");
    for(auto* stackFrame = static_cast<StackFrame*>(__builtin_frame_address(0)); 
        reinterpret_cast<uintptr_t>(stackFrame->previous)>=0xC0000000 ;
        stackFrame = stackFrame->previous)
      io::printf("  address:0x%lx\n", stackFrame->eip);
  }

  [[noreturn]] inline void panic(const char* format, ...) 
  { 
    io::print("Panic:\n  ");

    va_list ap;
    va_start(ap, format);
    core::io::vprintf(format, ap);
    va_end(ap);

    traceStackFrame();

    for(;;) asm("hlt"); 
  }
}
