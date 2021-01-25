#pragma once

#include <stdint.h>
#include <stddef.h>

namespace common::tasks
{
  /*
   * Copied verbatim from elf specification
   */
  using Elf32_Addr = uint32_t;
  using Elf32_Half = uint16_t;
  using Elf32_Off  = uint32_t;

  using Elf32_Sword = int32_t;
  using Elf32_Word  = uint32_t;


  enum ElfIdentIndex { EI_MAG0 = 0, EI_MAG1 = 1, EI_MAG2 = 2, EI_MAG3 = 3, EI_CLASS = 4, EI_DATA = 5, EI_VERSION = 6, EI_PAD = 7, EI_NIDENT = 16 };
  enum ElfClass { ELFCLASSNONE = 0, ELFCLASS32 = 1, ELFCLASS64 = 2 };
  enum ElfData  { ELFDATANONE = 0, ELFDATA2LSB = 1, ELFDATA2MSB = 2 };
  enum ElfVersion { EV_NONE = 0, EV_CURRENT = 1 };

  enum Elf32Type { ET_NONE = 0, ET_REL  = 1, ET_EXEC = 2, ET_DYN  = 3, ET_CORE = 4, ET_LOPROC = 0xFF00, ET_HIPROC = 0xFFFF };
  enum Elf32Machine { EM_NONE = 0, EM_M32  = 1, EM_SPARC = 2, EM_386 = 3, EM_68K = 4, EM_88K = 5, EM_860 = 7, EM_MIPS = 8 };

  enum Elf32PageFlags { PF_X = 0x1, PF_W = 0x2, PF_R = 0x4, PF_MASKPROC = 0xF0000000 };

  struct Elf32Header
  {
    uint8_t    ident[EI_NIDENT];
    Elf32_Half type;
    Elf32_Half machine;
    Elf32_Word version;
    Elf32_Addr entry;

    // Program header offset
    Elf32_Off  phoff;

    // Section header offset
    Elf32_Off  shoff;
    Elf32_Word flags;
    Elf32_Half ehsize;

    // Program header table
    Elf32_Half phentsize;
    Elf32_Half phnum;

    // Section header table
    Elf32_Half shentsize;
    Elf32_Half shnum;

    // Section Name String Table Index
    Elf32_Half shstrndx;
  };

  struct ELF32ProgramHeader
  {
    Elf32_Word type;
    Elf32_Off  offset;
    Elf32_Addr vaddr;
    Elf32_Addr paddr;
    Elf32_Word filesz;
    Elf32_Word memsz;
    Elf32_Word flags;
    Elf32_Word align;
  };

  inline bool checkDataRange(const char* data, size_t length, Elf32_Off off, Elf32_Word size)
  {
    Elf32_Addr result;
    if(__builtin_add_overflow(off, size, &result))
      return false;
    return result<=length;
  }

  inline const Elf32Header* getElf32Header(const char* data, size_t length)
  {
    if(!checkDataRange(data, length, 0, sizeof(Elf32Header)))
      return nullptr;

    const auto* header = reinterpret_cast<const Elf32Header*>(data);
    if(header->ident[EI_MAG0] != 0x7f || header->ident[EI_MAG1] != 'E'  || header->ident[EI_MAG2] != 'L'  || header->ident[EI_MAG3] != 'F')
      return nullptr;

    if(header->ident[EI_CLASS] != ELFCLASS32)
      return nullptr;

    if(header->ident[EI_DATA] != ELFDATA2LSB)
      return nullptr;

    if(header->ident[EI_VERSION] != EV_CURRENT)
      return nullptr;

    return header;
  }

  inline const ELF32ProgramHeader* getElf32ProgramHeaders(const char* data, size_t length, const Elf32Header* header, size_t& count)
  {
    if(!checkDataRange(data, length, header->phoff, static_cast<Elf32_Word>(header->phnum) * static_cast<Elf32_Word>(header->phentsize)))
      return nullptr;

    count = header->phnum;
    return reinterpret_cast<const ELF32ProgramHeader*>(data + header->phoff);
  }
}
