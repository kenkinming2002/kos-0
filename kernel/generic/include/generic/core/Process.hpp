#pragma once

#include <i686/core/memory/MemoryMapping.hpp>
#include <i686/core/Syscall.hpp>


#include <boost/intrusive/slist.hpp>

namespace core
{
  struct ProcessContext
  {
    uintptr_t esp;
    memory::MemoryMapping memoryMapping;
  };

  class Process
  {
  public:
    Process(uintptr_t kernelStack, uintptr_t startAddress);

  public:
    void addSection(memory::virtaddr_t virtualAddress, memory::Access access, memory::Permission permission,
        const uint8_t* content, size_t length);

  public:
    [[noreturn]] void enterUserMode() const;

  public:
    /*
     * Set stack as stack used for interrupt and syscall
     */
    void setStackAsActive() const;

  public:
    ProcessContext context;

    uintptr_t startAddress;
    uintptr_t kernelStack;
    uint16_t  kernelStackSegmentSelector;

  public:
    boost::intrusive::slist_member_hook<> slist_hook;
  };
}
