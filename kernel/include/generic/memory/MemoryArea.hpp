#pragma once

#include <generic/vfs/File.hpp>
#include <generic/memory/Page.hpp>

#include <common/i686/memory/Paging.hpp>

#include <sys/Types.hpp>

#include <librt/SharedPtr.hpp>

namespace core::memory
{
  struct MemoryArea
  {
  public:
    constexpr MemoryArea(uintptr_t addr, size_t length, Prot prot, rt::SharedPtr<vfs::File> file, size_t offset, MapType type)
      : addr(addr), length(length), prot(prot), file(rt::move(file)), offset(offset), type(type) {}

  public:
    MemoryArea clone();

  public:
    Result<physaddr_t> getPageFrame(uintptr_t addr);
    Result<physaddr_t> getWritablePageFrame(uintptr_t addr);
    void removePageFrame(uintptr_t addr);

  public:
    uintptr_t addr;
    size_t length;

    Prot prot;

    rt::SharedPtr<vfs::File> file;
    size_t offset;

    MapType type;

  public:
    rt::containers::Map<uintptr_t, rt::SharedPtr<Page>> pages;
  };

}
