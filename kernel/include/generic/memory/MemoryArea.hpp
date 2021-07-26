#pragma once

#include <generic/vfs/File.hpp>
#include <generic/memory/Page.hpp>
#include <generic/memory/Memory.hpp>

#include <sys/Types.hpp>

#include <librt/SharedPtr.hpp>

namespace core::memory
{
  struct MemoryArea
  {
  public:
    constexpr MemoryArea(uintptr_t addr, size_t length, Prot prot, rt::SharedPtr<vfs::File> file, size_t fileOffset, size_t fileLength, MapType type)
      : addr(addr), length(length), prot(prot), file(rt::move(file)), fileOffset(fileOffset), fileLength(fileLength), type(type)
    {
      ASSERT(addr   % PAGE_SIZE == 0);
      ASSERT(length % PAGE_SIZE == 0);
    }

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
    size_t fileOffset;
    size_t fileLength;

    MapType type;

  private:
    rt::containers::Map<uintptr_t, rt::SharedPtr<Page>> m_pages;
  };

}
