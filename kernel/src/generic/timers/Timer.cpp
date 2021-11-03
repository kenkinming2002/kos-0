#include <generic/timers/Timer.hpp>

#include <librt/Global.hpp>

namespace core::timers
{
  namespace
  {
    constinit rt::Global<rt::containers::IntrusiveList<Timer>> timers;
  }

  void archInitialize();
  void initialize()
  {
    timers.construct();
    archInitialize();
  }

  void registerTimer(Timer& timer)
  {
    timers().insert(timers().end(), timer);
  }

  void deregisterTimer(Timer& timer)
  {
    auto it = rt::find_if(timers().begin(), timers().end(), [&timer](Timer& _timer){
        return timer.name() == _timer.name();
    });

    ASSERT(it != timers().end());
    timers().remove_and_dispose(it, [](auto ptr) {});
  }

  Timer* getSchedulerTimer()
  {
    if(auto* timer = lookupTimer("APICTimer"))
      return timer;

    if(auto* timer = lookupTimer("PIT"))
      return timer;

    return nullptr;
  }

  Timer* lookupTimer(rt::StringRef name)
  {
    auto it = rt::find_if(timers().begin(), timers().end(), [&name](Timer& timer){
        return timer.name() == name;
    });
    return it != timers().end() ? &(*it) : nullptr;
  }
}


