#pragma once

#include <generic/memory/Virtual.hpp>

namespace core
{
  struct LocalAPIC
  {
    static constexpr size_t SPURIOUS_INTERRUPT_VECTOR_REGISTER = 0x0F0;
    static constexpr size_t EOI_REGISTER                       = 0x0B0;

    static constexpr size_t LVT_TIMER_REGISTER                  = 0x320;
    static constexpr size_t TIMER_INITIAL_COUNT_REGISTER        = 0x380;
    static constexpr size_t TIMER_CURRENT_COUNT_REGISTER        = 0x390;
    static constexpr size_t TIMER_DIVIDE_CONFIGURATION_REGISTER = 0x3E0;

    static void initialize();
    static void write(size_t offset, uint32_t data);
    static uint32_t read(size_t offset);
  };
}
