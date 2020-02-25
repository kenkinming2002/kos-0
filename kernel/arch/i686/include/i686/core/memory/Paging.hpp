#pragma once

#include <stdint.h>
#include <stddef.h>

#include <generic/core/memory/MemoryRegion.hpp>

namespace core::memory
{
  enum class CacheMode  { ENABLED, DISABLED };
  enum class WriteMode  { WRITE_THROUGH, WRITE_BACK };
  enum class Access     { ALL, SUPERVISOR_ONLY };
  enum class Permission { READ_ONLY, READ_WRITE };
  enum class TLBMode    { GLOBAL, LOCAL };
  enum class PageSize   { LARGE, NORMAL };

  class PageDirectoryEntry
  {
  public:
    PageDirectoryEntry() = default;
    PageDirectoryEntry(uint32_t address, CacheMode cacheMode, WriteMode
        writeMode, Access access, Permission permission, PageSize pageSize =
        PageSize::NORMAL);

  public:
    void clear();

  public:
    bool present() const;
    uint32_t address() const;

  public:
    void present(bool present);
    void address(uint32_t address);

  private:
    uint32_t m_data;
  } __attribute((packed));

  class PageTableEntry
  {
  public:
    PageTableEntry() = default;
    PageTableEntry(uint32_t address, TLBMode tlbMode, CacheMode cacheMode, WriteMode writeMode, Access access, Permission permission);

  public:
    void clear();

  public:
    bool present() const;
    uint32_t address() const;

  public:
    void present(bool present);
    void address(uint32_t address);

  public:
    uint32_t m_data;
  };
 
  static constexpr size_t PAGE_DIRECTORY_ENTRY_COUNT = 1024;
  static constexpr size_t PAGE_TABLE_ENTRY_COUNT = 1024;

  using PageDirectory = PageDirectoryEntry[PAGE_DIRECTORY_ENTRY_COUNT];
  using PageTable     = PageTableEntry[PAGE_TABLE_ENTRY_COUNT];
}
