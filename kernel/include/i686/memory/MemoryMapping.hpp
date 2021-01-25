#pragma once

#include <common/i686/memory/Paging.hpp>
#include <generic/memory/Memory.hpp>

#include <optional>
  
namespace core::memory
{
  /* 
   * Each class MemoryMapping manages the virtual to physical address translation of a single process.
   *
   * On i686 platform, the virtual address range are splited into multiple parts
   * [0GiB, 3GiB) - User Mappings(User code, data, etc.)
   * [3GiB, 4GiB) - Kernel Mappings(Kernel code, data, heap, etc.)
   *
   * The user mappings by default is private to each task but can also be
   * shared among different tasks.
   *
   * The kernel mappings is the same for every process, currently, there is no
   * mitigation for meltdown or spectre so the entirety of kernel and kernel
   * heap is always mapped in.
   */
  class MemoryMapping
  {
  public:
    static MemoryMapping* current;

  public:
    static std::optional<MemoryMapping> allocate();

  public:
    constexpr MemoryMapping(common::memory::PageDirectory* pageDirectory) : m_pageDirectory(pageDirectory) {}
    ~MemoryMapping();

  public:
    MemoryMapping(MemoryMapping&& other);
    MemoryMapping& operator=(MemoryMapping&& other);

  public:
    static uintptr_t doFractalMapping(uintptr_t phyaddr, size_t length);

  public:
    void synchronize();
    void makeCurrent();

  public:
    // This is the API we currently have
    void map(Pages virtualPages, common::memory::Access access, common::memory::Permission permission, std::optional<Pages> physicalPages = std::nullopt);
    void unmap(Pages virtualPages);

  public:
    uintptr_t virtualToPhysical(uintptr_t virtaddr);

  private:
    common::memory::PageDirectoryEntry& pageDirectoryEntry(size_t virtualIndex, bool allocate);
    common::memory::PageTableEntry& pageTableEntry(size_t virtualIndex, bool allocate);

  private:
    common::memory::PageDirectory* m_pageDirectory;
  };
}
