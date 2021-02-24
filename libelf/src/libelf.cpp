#include <libelf/libelf.hpp>

namespace elf32
{
  bool checkDataRange(const char* data, size_t length, Elf32_Off off, Elf32_Word size)
  {
    Elf32_Addr result;
    if(__builtin_add_overflow(off, size, &result))
      return false;
    return result<=length;
  }

  const Elf32_Ehdr* readHeader(const char* data, size_t length)
  {
    if(!checkDataRange(data, length, 0, sizeof(Elf32_Ehdr)))
      return nullptr;

    const auto* header = reinterpret_cast<const Elf32_Ehdr*>(data);
    if(header->e_ident[EI_MAG0] != 0x7f || header->e_ident[EI_MAG1] != 'E'  || header->e_ident[EI_MAG2] != 'L'  || header->e_ident[EI_MAG3] != 'F')
      return nullptr;

    if(header->e_ident[EI_CLASS] != ELFCLASS32)
      return nullptr;

    if(header->e_ident[EI_DATA] != ELFDATA2LSB)
      return nullptr;

    if(header->e_ident[EI_VERSION] != EV_CURRENT)
      return nullptr;

    return header;
  }

  const Elf32_Phdr* readProgramHeaders(const char* data, size_t length, const Elf32_Ehdr* header, size_t& count)
  {
    if(!checkDataRange(data, length, header->e_phoff, static_cast<Elf32_Word>(header->e_phnum) * static_cast<Elf32_Word>(header->e_phentsize)))
      return nullptr;

    count = header->e_phnum;
    return reinterpret_cast<const Elf32_Phdr*>(data + header->e_phoff);
  }
}
