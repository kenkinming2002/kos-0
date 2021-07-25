#include <boot/generic/Kernel.hpp>

#include <boot/generic/Memory.hpp>
#include <boot/generic/BootInformation.hpp>
#include <boot/i686/Paging.hpp>

#include <libelf/libelf.hpp>

#include <librt/Panic.hpp>
#include <librt/Iterator.hpp>
#include <librt/StringRef.hpp>
#include <librt/Log.hpp>
#include <librt/Strings.hpp>
#include <librt/Algorithm.hpp>

namespace boot
{
  extern "C" { uintptr_t entryAddress; }

  namespace
  {
    const Elf32_Ehdr* m_header;
    const Elf32_Phdr* m_programHeaders;
    size_t            m_programHeadersCount;

    const char* m_data;
    size_t m_length;
  }

  static void findKernelImage(struct multiboot_boot_information* multiboot2BootInformation)
  {
    for(auto* tag = multiboot2BootInformation->tags; tag->type != MULTIBOOT_TAG_TYPE_END; tag = multiboot2::next_tag(tag))
      if(tag->type == MULTIBOOT_TAG_TYPE_MODULE)
      {
        auto* module_tag = reinterpret_cast<struct multiboot_tag_module*>(tag);
        if(rt::strncmp(module_tag->cmdline, "kernel", 6) == 0)
        {
          m_data   = reinterpret_cast<const char*>(module_tag->mod_start);
          m_length = module_tag->mod_end-module_tag->mod_start;
          m_header = elf32::readHeader(m_data, m_length);
          if(!m_header)
            rt::panic("invalid ELF header\n");

          m_programHeaders = elf32::readProgramHeaders(m_data, m_length, m_header, m_programHeadersCount);
          if(!m_programHeaders)
            rt::panic("invalid ELF program headers\n");

          return;
        }
      }

    rt::panic("Kernel ELF image no found, it should be loaded as a multiboot2 module with cmdline kernel\n");
  }

  static void extractKernel()
  {
    using namespace boot::memory;
    using namespace common::memory;

    for(size_t i=0; i<m_programHeadersCount; ++i)
    {
      const auto& programHeader = m_programHeaders[i];
      if(programHeader.p_type == PT_LOAD)
      {
        if(!elf32::checkDataRange(m_data, m_length, programHeader.p_offset, programHeader.p_filesz))
          rt::panic("Malformed kernel binary\n");

        if(programHeader.p_filesz>programHeader.p_memsz)
          rt::panic("Malformed kernel binary\n");

        if(!(programHeader.p_flags & PF_R))
          rt::panic("Malformed kernel binary\n");

        auto pagesPermission = (programHeader.p_flags & PF_W) ? Permission::READ_WRITE : Permission::READ_ONLY;

        const char* fileSegment = m_data+programHeader.p_offset;
        char* segment = static_cast<char*>(memory::alloc(programHeader.p_memsz));
        rt::fill(segment    , segment     + programHeader.p_memsz , 0);
        rt::copy(fileSegment, fileSegment + programHeader.p_filesz, segment);
        memory::map(reinterpret_cast<uintptr_t>(segment), programHeader.p_vaddr, programHeader.p_memsz, Access::SUPERVISOR_ONLY, pagesPermission);
      }
    }

    entryAddress = m_header->e_entry;
  }

  void initializeKernel(struct multiboot_boot_information* multiboot2BootInformation)
  {
    findKernelImage(multiboot2BootInformation);
    extractKernel();
  }
}

