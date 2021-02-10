#include <x86/assembly/msr.hpp>

namespace core::assembly
{
  void wrmsr(uint32_t reg, uint64_t value)
  {
    uint32_t valueHigh = value >> 32;
    uint32_t valueLow  = value & 0xFFFFFFFF;
    asm volatile ("wrmsr" : : "c"(reg), "d"(valueHigh), "a"(valueLow));
  }

  uint64_t rdmsr(uint32_t reg)
  {
    uint32_t valueHigh;
    uint32_t valueLow;
    asm volatile ("rdmsr" : "=d"(valueHigh), "=a"(valueLow) : "c"(reg));
    return static_cast<uint64_t>(valueHigh) << 32 | static_cast<uint64_t>(valueLow);
  }
}
