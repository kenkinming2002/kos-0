#include <generic/timers/Timer.hpp>

namespace core::timers
{
  void initializePIT();
  void initializeAPICTimer();

  void archInitialize()
  {
    initializePIT();
    initializeAPICTimer();
  }
}
