#pragma once

#include <generic/memory/Memory.hpp>
#include <generic/memory/MemoryArea.hpp>

#include <common/i686/memory/Paging.hpp>

#include <librt/containers/List.hpp>
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

  using namespace common::memory;
  class MemoryMapping : public rt::SharedPtrHook
  {
  public:
    static rt::SharedPtr<MemoryMapping> current;

  public:
    static void initialize();

  public:
    static void makeCurrent(rt::SharedPtr<MemoryMapping> memoryMapping);

  public:
    static rt::SharedPtr<MemoryMapping> allocate();
    rt::SharedPtr<MemoryMapping> clone();

  public:
    MemoryMapping(PageDirectory* pageDirectory);
    ~MemoryMapping();

  public:
    Result<void> map(uintptr_t addr, size_t length, Prot prot, rt::SharedPtr<vfs::File> file = nullptr, size_t offset = 0);
    Result<void> unmap(uintptr_t addr, size_t length);
    Result<void> remap(uintptr_t addr, size_t length, size_t newLength);

  private:
    void mapReadonly(MemoryArea& memoryArea);
    void mapWritable(MemoryArea& memoryArea);
    void unmap(MemoryArea& memoryArea);
    void remap(MemoryArea& memoryArea, size_t newLength);

  private:
    void mapReadonlySingle(MemoryArea& memoryArea, uintptr_t addr);
    void mapWritableSingle(MemoryArea& memoryArea, uintptr_t addr);
    void unmapSingle(MemoryArea& memoryArea, uintptr_t addr);

  public:
    Result<void> handlePageFault(uintptr_t addr, uword_t errorCode);
    Result<void> handlePageFault(MemoryArea& memoryArea, size_t addr, uword_t errorCode);

  private:
    rt::containers::List<MemoryArea> m_memoryAreas;

  private:
    PageDirectory* m_pageDirectory;
  };
}
