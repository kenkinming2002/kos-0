#pragma once

#include <librt/FunctionRef.hpp>

namespace core
{
  void foreachCPUInitCall(rt::FunctionRef<void()> func);
  void foreachCPUInitHandleOnce();
  [[noreturn]] void foreachCPUInitHandleLoop();
}
