#include <librt/Trace.hpp>

#include <librt/Log.hpp>

#include <stdint.h>
#include <stddef.h>

namespace rt
{
  struct StackFrame
  {
    StackFrame* previous;
    uintptr_t eip;
  };

  constexpr static size_t MAX_STACK_FRAME = 8;
  void traceStackFrame()
  {
    log("StackTrace\n");

    const auto* stackFrame = static_cast<StackFrame*>(__builtin_frame_address(0));
    for(size_t i=0; i<MAX_STACK_FRAME; ++i)
    {
      logf("  address:0x%lx\n", stackFrame->eip);
      if(!stackFrame->previous)
        return;

      stackFrame = stackFrame->previous;
    }
  }
}
