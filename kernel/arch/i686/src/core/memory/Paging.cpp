#include <i686/core/memory/Paging.hpp>

namespace core::memory
{
  PageDirectoryEntry::PageDirectoryEntry() : m_data(0) {}
  PageDirectoryEntry::PageDirectoryEntry(uint32_t address, CacheMode cacheMode,
      WriteMode writeMode, Access access, Permission permission, PageSize pageSize)
    : m_data(0)
  {
    m_data |= address & 0xFFFFF000;
    if(pageSize   == PageSize::LARGE)          m_data |= 1u<<7;
    if(cacheMode  == CacheMode::DISABLED)      m_data |= 1u<<4;
    if(writeMode  == WriteMode::WRITE_THROUGH) m_data |= 1u<<3;
    if(access     == Access::ALL)              m_data |= 1u<<2;
    if(permission == Permission::READ_WRITE)   m_data |= 1u<<1;
    m_data |= 1u; // Present
  }
  
  bool PageDirectoryEntry::present() const
  {
    return m_data & 1u;
  }

  uint32_t PageDirectoryEntry::address() const
  {
    return m_data & 0xFFFFF000;
  }

  void PageDirectoryEntry::present(bool present)
  {
    m_data = (m_data & ~1u) | static_cast<uint32_t>(present);
  }

  void PageDirectoryEntry::address(uint32_t address)
  {
    m_data = (m_data & ~0xFFFFF000) | static_cast<uint32_t>(address);
  }

  PageTableEntry::PageTableEntry() : m_data(0) {}
  PageTableEntry::PageTableEntry(uint32_t address, TLBMode tlbMode, CacheMode cacheMode, WriteMode writeMode, Access access, 
      Permission permission)
  {
    m_data = address & 0xFFFFF000;
    if(tlbMode    == TLBMode::GLOBAL)          m_data |= 1u<<8;
    if(cacheMode  == CacheMode::DISABLED)      m_data |= 1u<<4;
    if(writeMode  == WriteMode::WRITE_THROUGH) m_data |= 1u<<3;
    if(access     == Access::ALL)              m_data |= 1u<<2;
    if(permission == Permission::READ_WRITE)   m_data |= 1u<<1;
    m_data |= 1u; // Present
  }

  bool PageTableEntry::present() const
  {
    return m_data & 1u;
  }

  uint32_t PageTableEntry::address() const
  {
    return m_data & 0xFFFFF000;
  }

  void PageTableEntry::present(bool present)
  {
    m_data = (m_data & ~1u) | static_cast<uint32_t>(present);
  }

  void PageTableEntry::address(uint32_t address)
  {
    m_data = (m_data & ~0xFFFFF000) | static_cast<uint32_t>(address);
  }
}
