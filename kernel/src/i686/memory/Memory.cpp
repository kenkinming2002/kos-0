#include <i686/memory/Memory.hpp>

#include <generic/memory/Memory.hpp>
#include <generic/BootInformation.hpp>

namespace core::memory
{
  void archInitialize()
  {
    using namespace common::memory;
    /* Map physical memory [0,size) to
     * [PHYSICAL_MEMEMORY_MAPPING_START, PHYSICAL_MEMEMORY_MAPPING_END) using
     * 4MiB mapping so that they are always accessible. Referede to as lowmem
     * in linux kernel, though I have no intent to support highmem. */
    physaddr_t phyaddr = 0;
    auto& pageDirectory = *bootInformation->pageDirectory;
    for(size_t i = PHYSICAL_MEMEMORY_MAPPING_START / LARGE_PAGE_SIZE; i != PHYSICAL_MEMEMORY_MAPPING_END / LARGE_PAGE_SIZE; ++i)
    {
      pageDirectory[i] = PageDirectoryEntry(phyaddr, CacheMode::ENABLED, WriteMode::WRITE_BACK, Access::SUPERVISOR_ONLY, Permission::READ_WRITE, PageSize::LARGE);
      asm volatile ( "invlpg [%[addr]]" : : [addr]"r"(i * LARGE_PAGE_SIZE) : "memory");

      phyaddr += LARGE_PAGE_SIZE;
    }
  }
}
