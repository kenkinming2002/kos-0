#pragma once

#include <common/i686/memory/Paging.hpp>

#include <generic/memory/Memory.hpp>
#include <generic/vfs/File.hpp>
#include <generic/memory/Pages.hpp>
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
  struct MemoryArea : rt::SharedPtrHook
  {
  public:
    enum class Type { PRIVATE, SHARED };

  public:
    constexpr MemoryArea(uintptr_t addr, size_t length, Permission permission, rt::SharedPtr<vfs::File> file, size_t offset, Type type)
      : addr(addr), length(length), permission(permission), file(rt::move(file)), offset(offset), type(type) {}

  public:
    uintptr_t addr;
    size_t length;

    Permission permission;

    rt::SharedPtr<vfs::File> file;
    size_t offset;

    Type type;
  };

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
    MemoryMapping(PageDirectory* pageDirectory);
    ~MemoryMapping();


  public:
    Result<void> map(uintptr_t addr, size_t length, Permission permission, rt::SharedPtr<vfs::File> file = nullptr, size_t offset = 0);

  private:
    void map(MemoryArea& memoryArea);
    void mapSingle(MemoryArea& memoryArea, uintptr_t addr, size_t offset);

  public:
    Result<void> unmap(uintptr_t addr, size_t length);

  private:
    void unmap(MemoryArea& memoryArea);
    void unmapSingle(MemoryArea& memoryArea, uintptr_t addr);

  public:
    Result<void> remap(uintptr_t addr, size_t length, size_t newLength);
    void remap(MemoryArea& memoryArea, size_t newLength);

  private:
    rt::containers::List<rt::SharedPtr<MemoryArea>> m_memoryAreas;

  private:
    PageDirectory* m_pageDirectory;
  };
}
