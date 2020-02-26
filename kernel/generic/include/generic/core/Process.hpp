#pragma once

#include <i686/core/memory/MemoryMapping.hpp>

namespace core
{
  class Process
  {
  public:
    Process();

  public:
    void setAsActive() const;

  public:
    void addSection(memory::virtaddr_t virtualAddress, memory::Access access, memory::Permission permission,
        const uint8_t* content, size_t length);

  public:
    [[noreturn]] void run() const;

  private:
    memory::MemoryMapping m_memoryMapping; // Each process have its own memory mapping
    uintptr_t m_kernelStack;
    uint16_t  m_kernelStackSegmentSelector;
  };
}
