#pragma once

#include <sys/Types.hpp>

namespace core::tasks
{
  struct SchedInfo
  {
    unsigned cpuid;
    pid_t pid;

    enum class State { RUNNING, RUNNABLE, BLOCKED } state;

    bool pendingKill = false;
    status_t status = 0;
  };
}
