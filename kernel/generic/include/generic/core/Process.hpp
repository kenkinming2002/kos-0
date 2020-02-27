#pragma once

#include <i686/core/memory/MemoryMapping.hpp>
#include <i686/core/Syscall.hpp>


#include <boost/intrusive/slist.hpp>

namespace core
{
  class Process : public boost::intrusive::slist_base_hook<>
  {
  public:
    Process(uintptr_t startAddress);

  public:
    void setAsActive() const;

  public:
    void addSection(memory::virtaddr_t virtualAddress, memory::Access access, memory::Permission permission,
        const uint8_t* content, size_t length);

  public:
    [[noreturn]] void run() const;

  public:
    void state(const State& state) { m_state = state; }

  private:
    memory::MemoryMapping m_memoryMapping; // Each process have its own memory mapping
    uintptr_t m_kernelStack;
    uint16_t  m_kernelStackSegmentSelector;

  private:
    State m_state;
  };
}
