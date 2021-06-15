#pragma once

#include <common/i686/memory/Paging.hpp>
#include <generic/memory/Memory.hpp>

#include <librt/SharedPtr.hpp>
#include <librt/NonCopyable.hpp>

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
  class MemoryMapping : public rt::SharedPtrHook
  {
  public:
    static void initialize();

  public:
    static rt::SharedPtr<MemoryMapping> current();
    static void makeCurrent(rt::SharedPtr<MemoryMapping> memoryMapping);

  public:
    static rt::SharedPtr<MemoryMapping> allocate();

  public:
    MemoryMapping(common::memory::PageDirectory* pageDirectory);
    ~MemoryMapping();

  public:
    void map(Pages virtualPages, common::memory::Access access, common::memory::Permission permission, rt::Optional<Pages> physicalPages = rt::nullOptional);
    void unmap(Pages virtualPages);

  private:
    common::memory::PageDirectoryEntry& pageDirectoryEntry(size_t virtualIndex, bool allocate);
    common::memory::PageTableEntry& pageTableEntry(size_t virtualIndex, bool allocate);

  private:
    common::memory::PageDirectory* m_pageDirectory;
  };
}
