#pragma once

#include <common/i686/memory/Paging.hpp>
#include <core/generic/memory/Memory.hpp>

#include <optional>
  
namespace core::memory
{
  /* 
   * The last of 1024 Page Directory Entries is used for temporary mapping and
   * reserved. This may seem wasteful, but virtual address space is cheap and
   * 4MiB out of 4MiB is not going to matter much. This is *NOT* going to be
   * synchronized between different MemoryMapping like
   *
   * Memory Mapping is non-owning. We do not do any form of reference counting.
   */
  class MemoryMapping
  {
  public:
    static MemoryMapping  initial;
    static MemoryMapping* current;

  public:
    friend void initMemoryMapping();

  private:
    static common::memory::PageDirectoryEntry kernelPageDirectoryEntries[256];

  public:
    static std::optional<MemoryMapping> allocate();

  public:
    MemoryMapping(uintptr_t pageDirectoryPhysicalAddress, uintptr_t pageDirectoryVirtualAddress);
    ~MemoryMapping();

  public:
    MemoryMapping(MemoryMapping&& other);
    MemoryMapping& operator=(MemoryMapping&& other);

  public:
    static uintptr_t doFractalMapping(uintptr_t phyaddr, size_t length);

  public:
    void makeCurrent();

  public:
    void map(Pages physicalPages, Pages virtualPages, common::memory::Access access, common::memory::Permission permission);
    Pages unmap(Pages virtualPages);

  public:
    uintptr_t virtualToPhysical(uintptr_t virtaddr);

  private:
    auto& pageDirectory() { return *reinterpret_cast<common::memory::PageDirectory*>(m_pageDirectoryVirtualAddress); }
    const auto& pageDirectory() const { return *reinterpret_cast<common::memory::PageDirectory*>(m_pageDirectoryVirtualAddress); }

  private:
    uintptr_t m_pageDirectoryPhysicalAddress;
    uintptr_t m_pageDirectoryVirtualAddress;
  };
}
