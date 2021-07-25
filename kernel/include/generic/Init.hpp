#pragma once

namespace core
{
  using init_func_t = void(*)();

  void foreachCPUInitCall(init_func_t func);
  void foreachCPUInitHandleOnce();
  [[noreturn]] void foreachCPUInitHandleLoop();
}
