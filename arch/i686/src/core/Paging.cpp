#include <i686/core/Paging.hpp>

PageDirectory::PageDirectory(unsigned address, char cached, char write_through, char user_access, char writable)
{
  m_data = 0x00000000;

  m_data |= address & 0xFFFFF000;
  //m_data |= 1<<7; // 4 MiB page
  if(!cached)
    m_data |= 1<<4;
  if(write_through)
    m_data |= 1<<3;
  if(user_access)
    m_data |= 1<<2;
  if(writable)
    m_data |= 1<<1;

  m_data |= 1; // Present
}
