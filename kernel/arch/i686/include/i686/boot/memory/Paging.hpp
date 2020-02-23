#pragma once

/**
 * @file include/i686/boot/Paging.hpp
 *
 * Same as include/i686/core/Paging.hpp, but all function are place in .boot.*
 * section for use at boot time before higher-half kernel is set-up and higher-
 * half kernel code is usable.
 */

#include <stdint.h>
#include <stddef.h>

#include <i686/boot/boot.hpp>

namespace boot::memory
{
  enum class CacheMode  { ENABLED, DISABLED };
  enum class WriteMode  { WRITE_THROUGH, WRITE_BACK };
  enum class Access     { ALL, SUPERVISOR_ONLY };
  enum class Permission { READ_ONLY, READ_WRITE };
  enum class TLBMode    { GLOBAL, LOCAL };

  class PageDirectoryEntry
  {
  public:
    BOOT_FUNCTION PageDirectoryEntry();
    BOOT_FUNCTION PageDirectoryEntry(uint32_t address, CacheMode cacheMode,
        WriteMode writeMode, Access access, Permission permission);

  public:
    BOOT_FUNCTION bool present() const;
    BOOT_FUNCTION uint32_t address() const;

  public:
    BOOT_FUNCTION void present(bool present);
    BOOT_FUNCTION void address(uint32_t address);

  private:
    uint32_t m_data;
  } __attribute((packed));

  class PageTableEntry
  {
  public:
    BOOT_FUNCTION PageTableEntry(); 
    BOOT_FUNCTION PageTableEntry(uint32_t address, TLBMode tlbMode, CacheMode
        cacheMode, WriteMode writeMode, Access access, Permission permission);

  public: 
    BOOT_FUNCTION bool present() const; 
    BOOT_FUNCTION uint32_t address() const;

  public: 
    BOOT_FUNCTION void present(bool present); 
    BOOT_FUNCTION void address(uint32_t address);

  private:
    uint32_t m_data;
  };

  constexpr size_t PAGE_DIRECTORY_ENTRY_COUNT = 1024;
  constexpr size_t PAGE_TABLE_ENTRY_COUNT = 1024;

  using PageDirectory = PageDirectoryEntry[PAGE_DIRECTORY_ENTRY_COUNT];
  using PageTable     = PageTableEntry[PAGE_TABLE_ENTRY_COUNT];
}
