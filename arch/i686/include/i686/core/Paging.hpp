#pragma once

class PageDirectory
{
public:
  PageDirectory() = default;
  PageDirectory(unsigned address, char cached, char write_through, char user_access, char writable);

private:
  unsigned m_data;
} __attribute((packed));

