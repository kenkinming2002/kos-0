#include <boot/generic/Kernel.hpp>

#include <boot/generic/multiboot2.hpp>
#include <boot/generic/Memory.hpp>
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
  Kernel::Kernel(BootInformation& bootInformation)
  {
    for(size_t i=0; i<bootInformation.moduleEntriesCount; ++i)
    {
      auto& moduleEntry = bootInformation.moduleEntries[i];
      if(moduleEntry.cmdline == rt::StringRef("kernel"))
      {
        m_data   = reinterpret_cast<char*>(moduleEntry.addr);
        m_length = moduleEntry.len;
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

  bool Kernel::extractAndMap(BootInformation& bootInformation)
  {
    using namespace boot::memory;
    using namespace common::memory;

    for(size_t i=0; i<m_programHeadersCount; ++i)
    {
      const auto& programHeader = m_programHeaders[i];

      if(programHeader.p_type == PT_LOAD)
      {
        if(!elf32::checkDataRange(m_data, m_length, programHeader.p_offset, programHeader.p_filesz))
          return false;

        if(programHeader.p_filesz>programHeader.p_memsz)
          return false;

        if(!(programHeader.p_flags & PF_R))
          return false; // We do not support a page that is not readable

        auto pagesPermission = (programHeader.p_flags & PF_W) ? Permission::READ_WRITE : Permission::READ_ONLY;

        const char* fileSegment = m_data+programHeader.p_offset;
        char* segment = static_cast<char*>(memory::alloc(bootInformation, programHeader.p_memsz, ReservedMemoryRegion::Type::KERNEL));
        rt::fill(segment    , segment     + programHeader.p_memsz , 0);
        rt::copy(fileSegment, fileSegment + programHeader.p_filesz, segment);
        memory::map(bootInformation, reinterpret_cast<uintptr_t>(segment), programHeader.p_vaddr, programHeader.p_memsz, Access::SUPERVISOR_ONLY, pagesPermission);
      }
    }

    return true;
  }

  void Kernel::run(BootInformation& bootInformation)
  {
    using namespace boot::memory;
    using namespace common::memory;

    auto* allocBootInformation = static_cast<BootInformation*>(memory::alloc(bootInformation, sizeof bootInformation, ReservedMemoryRegion::Type::BOOT_INFORMATION));
    *allocBootInformation = bootInformation;

    asm volatile ("jmp %[entry];" : : [entry]"r"(m_header->e_entry), "b"(reinterpret_cast<uintptr_t>(allocBootInformation)+bootInformation.physicalMemoryOffset) : "eax", "memory");
    __builtin_unreachable();
  }
}

