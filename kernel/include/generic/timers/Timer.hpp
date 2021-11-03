#pragma once

#include <librt/containers/IntrusiveList.hpp>
#include <librt/StringRef.hpp>

namespace core::timers
{
  class Timer : public rt::containers::IntrusiveListHook
  {
  public:
    typedef void(*callback_t)(void*);

  public:
    virtual rt::StringRef name() = 0;
    virtual void addCallback(callback_t callback, void* data) = 0;
    virtual void reset() = 0;

  public:
    virtual ~Timer() = default;
  };

  void initialize();

  void registerTimer(Timer& timer);
  void deregisterTimer(Timer& timer);

  /* Get a timer, suitable for scheduler usage
   * TODO: Do this with flags argument that specify timer characteristics
   *       instead of seperate function dedicated solely to scheduler */
  Timer* getSchedulerTimer();
  Timer* lookupTimer(rt::StringRef name);
}
