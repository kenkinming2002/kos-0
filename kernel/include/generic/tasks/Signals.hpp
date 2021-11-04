#pragma once

#include <librt/Assert.hpp>
#include <generic/SpinLock.hpp>

#include <stddef.h>

namespace core::tasks
{
  static constexpr size_t USER_SIG_MIN   = 0;
  static constexpr size_t USER_SIG_MAX   = 32;
  static constexpr size_t KERNEL_SIG_MIN = 32;
  static constexpr size_t KERNEL_SIG_MAX = 64;
  static constexpr size_t SIG_MAX        = 64;

  enum class Signal
  {
    NONE = -1,

    /* User signals */
    TERM = USER_SIG_MIN,
    KILL,

    /* Kernel signals */
    IO = KERNEL_SIG_MIN, /* Used internally to by async_wait syscall */
    MAX = SIG_MAX
  };

  struct Sigaction
  {
    bool pending : 1;
    bool kill    : 1;
  };

  struct SigInfo
  {
    constexpr SigInfo()
    {
      for(auto& action : actions)
        action = Sigaction{.pending = false, .kill = false};

      // Special hard coded signal
      actions[static_cast<unsigned>(Signal::TERM)].kill = true;
      actions[static_cast<unsigned>(Signal::KILL)].kill = true;
    }

    mutable core::SpinLock lock;
    Sigaction actions[SIG_MAX];

    bool pendingSignal() const
    {
      core::LockGuard guard(lock);
      for(const auto& action : actions)
        if(action.pending)
          return true;

      return false;
    }

    Signal get()
    {
      core::LockGuard guard(lock);
      for(size_t i=0; i<SIG_MAX; ++i)
        if(actions[i].pending)
        {
          actions[i].pending = false;
          return static_cast<Signal>(i);
        }

      ASSERT_UNREACHABLE;
    }
  };
}
