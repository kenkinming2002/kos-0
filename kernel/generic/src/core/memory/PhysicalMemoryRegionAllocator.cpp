#include <generic/core/memory/PhysicalMemoryRegionAllocator.hpp>

#include <generic/io/Print.hpp>

#include <algorithm>

#include <generic/core/memory/LinkedListMemoryRegionAllocator.hpp>

extern "C"
{
  extern std::byte kernel_virtual_start[];
  extern std::byte kernel_virtual_end[];

  extern std::byte kernel_physical_start[];
  extern std::byte kernel_physical_end[];
}

namespace core::memory
{
  template<typename Impl>
  PhysicalMemoryRegionAllocator<Impl>::PhysicalMemoryRegionAllocator(BootInformation::MemoryMapEntry* memoryMapEntries, size_t length) 
  {
    io::print("DEBUG: Initializing Physical Memory Manager\n");

    io::print("  Kernel physical address range: ", reinterpret_cast<uintptr_t>(kernel_physical_start), "-", reinterpret_cast<uintptr_t>(kernel_physical_end), "\n");
    auto kernelMemoryRegion = MemoryRegion(kernel_physical_start, kernel_physical_end);

    io::print("  Usable physical address ranges(including address range):\n");
    auto it = this->m_memoryRegions.before_begin();
    for(size_t i=0; i<length; ++i)
    {
      BootInformation::MemoryMapEntry& memoryMapEntry = memoryMapEntries[i];
      if(memoryMapEntry.type == BootInformation::MemoryMapEntry::Type::AVAILABLE)
      {
        io::print("   - ", memoryMapEntry.addr, "-", memoryMapEntry.addr + memoryMapEntry.len, "\n");
        auto [first, second] = MemoryRegion::difference(MemoryRegion(memoryMapEntry), kernelMemoryRegion);
        {
          if(first)
            it = this->m_memoryRegions.insert_after(it, *(new MemoryRegion(*first)));
          if(second)
            it = this->m_memoryRegions.insert_after(it, *(new MemoryRegion(*second)));
        }
      }
    }
  }

  template class PhysicalMemoryRegionAllocator<LinkedListMemoryRegionAllocator>;
}

