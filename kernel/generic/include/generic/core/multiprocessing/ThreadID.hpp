#pragma once

#include <stdint.h>
#include <stddef.h>

#include <limits>

namespace core::multiprocessing
{
  using ThreadID = int16_t;
  constexpr static size_t MAX_PROCESS_COUNT = std::numeric_limits<ThreadID>::max()-1;
  constexpr static ThreadID anyThreadID     = std::numeric_limits<ThreadID>::max();

  ThreadID allocThreadID();
  void freeThreadID(ThreadID tid);
}
