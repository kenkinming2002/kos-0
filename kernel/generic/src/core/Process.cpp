#include <generic/core/Process.hpp>

#include <i686/boot/boot.hpp>
#include <generic/core/Memory.hpp>

#include <generic/utils/Utilities.hpp>

#include <string.h>

namespace core
{
  Process::Process() : m_memoryMapping(memory::higherHalfMemoryMapping) {}

  void Process::setAsActive() const
  {
    m_memoryMapping.setAsActive();
  }

  void Process::addSection(memory::virtaddr_t virtualAddress, memory::Access access, memory::Permission permission,
      const uint8_t* content, size_t length)
  {
    size_t pageCount = (length + memory::PAGE_SIZE - 1) / memory::PAGE_SIZE;

    auto physicalMemoryRegion = core::memory::allocatePhysicalMemoryRegion(pageCount);
    if(!physicalMemoryRegion)
      return;

    auto virtualMemoryRegion = memory::MemoryRegion(virtualAddress, pageCount * memory::PAGE_SIZE, memory::MemoryRegion::address_length_tag);
    m_memoryMapping.map(*physicalMemoryRegion, virtualMemoryRegion, access, permission);

    // Memory is mapped here 
    memcpy(reinterpret_cast<void*>(virtualAddress), reinterpret_cast<const void*>(content), length);
  }
}
