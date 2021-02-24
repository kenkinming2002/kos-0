#pragma once

#include <stdint.h>
#include <stddef.h>

/*
 * Copied verbatim from elf specification
 */
typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef int32_t  Elf32_Sword;
typedef uint32_t Elf32_Word;

static constexpr auto EI_MAG0    = 0;
static constexpr auto EI_MAG1    = 1;
static constexpr auto EI_MAG2    = 2;
static constexpr auto EI_MAG3    = 3;
static constexpr auto EI_CLASS   = 4;
static constexpr auto EI_DATA    = 5;
static constexpr auto EI_VERSION = 6;
static constexpr auto EI_PAD     = 7;
static constexpr auto EI_NIDENT  = 16;

static constexpr auto ELFCLASSNONE = 0;
static constexpr auto ELFCLASS32   = 1;
static constexpr auto ELFCLASS64   = 2;
static constexpr auto ELFDATANONE  = 0;
static constexpr auto ELFDATA2LSB  = 1;
static constexpr auto ELFDATA2MSB  = 2;

static constexpr auto EV_NONE    = 0;
static constexpr auto EV_CURRENT = 1 ;

static constexpr auto ET_NONE   = 0;
static constexpr auto ET_REL    = 1;
static constexpr auto ET_EXEC   = 2;
static constexpr auto ET_DYN    = 3;
static constexpr auto ET_CORE   = 4;
static constexpr auto ET_LOPROC = 0xFF00;
static constexpr auto ET_HIPROC = 0xFFFF ;

static constexpr auto EM_NONE  = 0;
static constexpr auto EM_M32   = 1;
static constexpr auto EM_SPARC = 2;
static constexpr auto EM_386   = 3;
static constexpr auto EM_68K   = 4;
static constexpr auto EM_88K   = 5;
static constexpr auto EM_860   = 7;
static constexpr auto EM_MIPS  = 8 ;

static constexpr auto PT_NULL    = 0;
static constexpr auto PT_LOAD    = 1;
static constexpr auto PT_DYNAMIC = 2;
static constexpr auto PT_INTERP  = 3;
static constexpr auto PT_NOTE    = 4;
static constexpr auto PT_SHLIB   = 5;
static constexpr auto PT_PHDR    = 6;
static constexpr auto PT_LOPROC  = 0x70000000;
static constexpr auto PT_HIPROC  = 0x7FFFFFFF;

static constexpr auto PF_X        = 0x1;
static constexpr auto PF_W        = 0x2;
static constexpr auto PF_R        = 0x4;
static constexpr auto PF_MASKPROC = 0xF0000000 ;


struct Elf32_Ehdr
{
  unsigned char e_ident[EI_NIDENT];
  Elf32_Half    e_type;
  Elf32_Half    e_machine;
  Elf32_Word    e_version;
  Elf32_Addr    e_entry;
  Elf32_Off     e_phoff;
  Elf32_Off     e_shoff;
  Elf32_Word    e_flags;
  Elf32_Half    e_ehsize;
  Elf32_Half    e_phentsize;
  Elf32_Half    e_phnum;
  Elf32_Half    e_shentsize;
  Elf32_Half    e_shnum;
  Elf32_Half    e_shstrndx;
};

struct Elf32_Phdr
{
  Elf32_Word p_type;
  Elf32_Off  p_offset;
  Elf32_Addr p_vaddr;
  Elf32_Addr p_paddr;
  Elf32_Word p_filesz;
  Elf32_Word p_memsz;
  Elf32_Word p_flags;
  Elf32_Word p_align;
};

