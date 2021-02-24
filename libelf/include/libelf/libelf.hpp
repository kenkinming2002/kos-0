#pragma once

#include <libelf/Types.hpp>

namespace elf32
{
  bool checkDataRange(const char* data, size_t length, Elf32_Off off, Elf32_Word size);
  const Elf32_Ehdr* readHeader(const char* data, size_t length);
  const Elf32_Phdr* readProgramHeaders(const char* data, size_t length, const Elf32_Ehdr* header, size_t& count);
}
