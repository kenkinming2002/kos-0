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

  private:
    memory::MemoryMapping m_memoryMapping; // Each process have its own memory mapping
  };
}
