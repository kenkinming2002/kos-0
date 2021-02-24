#include <boot/generic/Kernel.hpp>

#include <boot/generic/multiboot2-Utils.hpp>
#include <boot/generic/Memory.hpp>
#include <boot/i686/Paging.hpp>

#include <libelf/libelf.hpp>

#include <librt/Panic.hpp>
#include <librt/Iterator.hpp>
#include <librt/Log.hpp>
#include <librt/Strings.hpp>
#include <librt/Algorithm.hpp>

namespace boot
{
  rt::Optional<Kernel> Kernel::from(const multiboot_boot_information* multiboot2BootInformation)
  {
    Kernel kernel;
    for(auto* tag = multiboot2BootInformation->tags; tag->type != MULTIBOOT_TAG_TYPE_END; tag = multiboot2::next_tag(tag))
      if(tag->type == MULTIBOOT_TAG_TYPE_MODULE)
      {
        auto* module_tag = reinterpret_cast<const struct multiboot_tag_module*>(tag);
        if(rt::strncmp(module_tag->cmdline, "kernel", 6) == 0)
        {
          kernel.m_data   = reinterpret_cast<char*>(module_tag->mod_start);
          kernel.m_length = module_tag->mod_end-module_tag->mod_start;
          kernel.m_header = elf32::readHeader(kernel.m_data, kernel.m_length);
          if(!kernel.m_header)
            return rt::nullOptional;

          kernel.m_programHeaders = elf32::readProgramHeaders(kernel.m_data, kernel.m_length, kernel.m_header, kernel.m_programHeadersCount);
          if(!kernel.m_programHeaders)
            return rt::nullOptional;

          return kernel;
        }
      }

    return rt::nullOptional;
  }

  int Kernel::extractAndMap(BootInformation& bootInformation)
  {
    using namespace boot::memory;
    using namespace common::memory;

    for(size_t i=0; i<m_programHeadersCount; ++i)
    {
      const auto& programHeader = m_programHeaders[i];

      if(programHeader.p_type == PT_LOAD)
      {
        if(!elf32::checkDataRange(m_data, m_length, programHeader.p_offset, programHeader.p_filesz))
          return -1;

        if(programHeader.p_filesz>programHeader.p_memsz)
          return -1;

        if(!(programHeader.p_flags & PF_R))
          return -1; // We do not support a page that is not readable

        auto pagesPermission = (programHeader.p_flags & PF_W) ? Permission::READ_WRITE : Permission::READ_ONLY;

        const char* fileSegment = m_data+programHeader.p_offset;
        char* segment = static_cast<char*>(memory::alloc(bootInformation, programHeader.p_memsz, ReservedMemoryRegion::Type::KERNEL));
        rt::fill(segment    , segment     + programHeader.p_memsz , 0);
        rt::copy(fileSegment, fileSegment + programHeader.p_filesz, segment);

        if(memory::map(bootInformation, reinterpret_cast<uintptr_t>(segment), programHeader.p_vaddr, programHeader.p_memsz, Access::SUPERVISOR_ONLY, pagesPermission) == MAP_FAILED)
          return -1;
      }
    }

    return 0;
  }

  void Kernel::run(BootInformation& bootInformation)
  {
    using namespace boot::memory;
    using namespace common::memory;

    auto* allocBootInformation = static_cast<BootInformation*>(memory::alloc(bootInformation, sizeof bootInformation, ReservedMemoryRegion::Type::BOOT_INFORMATION));
    *allocBootInformation = bootInformation;

    auto result = memory::map(bootInformation, reinterpret_cast<uintptr_t>(allocBootInformation), sizeof *allocBootInformation, Access::SUPERVISOR_ONLY, Permission::READ_WRITE);
    if(result == MAP_FAILED)
      rt::panic("Failed to map boot information\n");

    asm volatile (
        // Load page directory
        "jmp %[entry];"
        : : [entry]"r"(m_header->e_entry), "b"(result) : "eax", "memory"
        );
    __builtin_unreachable();
  }
}

