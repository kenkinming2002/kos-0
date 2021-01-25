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

#define FORCE_INLINE [[gnu::always_inline]]

namespace common::memory
{
  static constexpr size_t PAGE_SIZE       = 4096;
  static constexpr size_t LARGE_PAGE_SIZE = PAGE_SIZE * 1024;

  enum class CacheMode  { ENABLED, DISABLED };
  enum class WriteMode  { WRITE_THROUGH, WRITE_BACK };
  enum class Access     { ALL, SUPERVISOR_ONLY };
  enum class Permission { READ_ONLY, READ_WRITE };
  enum class TLBMode    { GLOBAL, LOCAL };
  enum class PageSize   { LARGE, NORMAL };

  class [[gnu::packed]] PageDirectoryEntry
  {
  public:
    FORCE_INLINE constexpr PageDirectoryEntry() = default;
    FORCE_INLINE constexpr PageDirectoryEntry(uint32_t address, CacheMode cacheMode, WriteMode writeMode, Access access, Permission permission, PageSize pageSize = PageSize::NORMAL)
    {
      m_data |= address & 0xFFFFF000;
      if(pageSize   == PageSize::LARGE)          m_data |= 1u<<7;
      if(cacheMode  == CacheMode::DISABLED)      m_data |= 1u<<4;
      if(writeMode  == WriteMode::WRITE_THROUGH) m_data |= 1u<<3;
      if(access     == Access::ALL)              m_data |= 1u<<2;
      if(permission == Permission::READ_WRITE)   m_data |= 1u<<1;
      m_data |= 1u; // Present
    }

  public:
    FORCE_INLINE constexpr bool present() const { return m_data & 1u; }
    FORCE_INLINE constexpr uint32_t address() const { return m_data & 0xFFFFF000; }

  public:
    FORCE_INLINE constexpr void present(bool present) { m_data = (m_data & ~1u) | static_cast<uint32_t>(present); }
    FORCE_INLINE constexpr void address(uint32_t address) { m_data = (m_data & ~0xFFFFF000) | static_cast<uint32_t>(address); }

  private:
    uint32_t m_data = 0;
  };

  class PageTableEntry
  {
  public:
    FORCE_INLINE constexpr PageTableEntry() = default;
    FORCE_INLINE constexpr PageTableEntry(uint32_t address, TLBMode tlbMode, CacheMode cacheMode, WriteMode writeMode, Access access, Permission permission)
    {
      m_data = address & 0xFFFFF000;
      if(tlbMode    == TLBMode::GLOBAL)          m_data |= 1u<<8;
      if(cacheMode  == CacheMode::DISABLED)      m_data |= 1u<<4;
      if(writeMode  == WriteMode::WRITE_THROUGH) m_data |= 1u<<3;
      if(access     == Access::ALL)              m_data |= 1u<<2;
      if(permission == Permission::READ_WRITE)   m_data |= 1u<<1;
      m_data |= 1u; // Present
    }

  public: 
    FORCE_INLINE constexpr bool present() const { return m_data & 1u; }
    FORCE_INLINE constexpr uint32_t address() const { return m_data & 0xFFFFF000; }

  public: 
    FORCE_INLINE constexpr void present(bool present) { m_data = (m_data & ~1u) | static_cast<uint32_t>(present); }
    FORCE_INLINE constexpr void address(uint32_t address) { m_data = (m_data & ~0xFFFFF000) | static_cast<uint32_t>(address); }

  public:
    uint32_t m_data = 0;
  };

  constexpr size_t PAGE_DIRECTORY_ENTRY_COUNT = 1024;
  constexpr size_t PAGE_TABLE_ENTRY_COUNT = 1024;

  using PageDirectory = PageDirectoryEntry[PAGE_DIRECTORY_ENTRY_COUNT];
  using PageTable     = PageTableEntry[PAGE_TABLE_ENTRY_COUNT];
}

