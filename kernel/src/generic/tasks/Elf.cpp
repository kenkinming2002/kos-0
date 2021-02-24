#include <generic/tasks/Elf.hpp>

#include <common/i686/memory/Paging.hpp>

#include <generic/memory/Pages.hpp>

#include <i686/memory/MemoryMapping.hpp>

#include <libelf/libelf.hpp>

#include <librt/Optional.hpp>
#include <librt/Algorithm.hpp>

namespace core::tasks
{
  static bool checkVirtualRange(Elf32_Off off, Elf32_Word size)
  {
    Elf32_Addr result;
    if(__builtin_add_overflow(off, size, &result))
      return false;
    return result<=0xC0000000;
  }

  static int loadProgramHeader(const char* data, size_t length, Task& task, const Elf32_Phdr& programHeader)
  {
    using namespace core::memory;
    using namespace common::memory;

    if(programHeader.p_type == PT_LOAD)
    {
      if(!elf32::checkDataRange(data, length, programHeader.p_offset, programHeader.p_filesz))
        return -1;

      if(!checkVirtualRange(programHeader.p_vaddr, programHeader.p_memsz))
        return -1;

      if(programHeader.p_filesz>programHeader.p_memsz)
        return -1; // It is better to be more pedantic in OS code

      // TODO: Gracefully handle any error
      if(!(programHeader.p_flags & PF_R))
        return -1; // We do not support a page that is not readable

      auto pagesPermission = (programHeader.p_flags & PF_W) ? Permission::READ_WRITE : Permission::READ_ONLY;
      task.memoryMapping()->map(Pages::fromAggressive(programHeader.p_vaddr, programHeader.p_vaddr+programHeader.p_memsz), Access::ALL, pagesPermission);

      auto fileSegmentBegin    = reinterpret_cast<const char*>(data+programHeader.p_offset);
      auto fileSegmentEnd      = reinterpret_cast<const char*>(data+programHeader.p_offset+programHeader.p_filesz);
      auto virtualSegmentBegin = reinterpret_cast<char*>(programHeader.p_vaddr);
      auto virtualSegmentEnd   = reinterpret_cast<char*>(programHeader.p_vaddr+programHeader.p_memsz);

      rt::fill(virtualSegmentBegin, virtualSegmentEnd, '\0');
      rt::copy(fileSegmentBegin, fileSegmentEnd, virtualSegmentBegin);
    }

    return 0;
  }

  int loadElf(Task& task, char* data, size_t length)
  {
    if(length<sizeof(Elf32_Ehdr))
      return -1;

    const auto* header = elf32::readHeader(data, length);
    if(!header)
      return -1;

    size_t count;
    const auto* programHeaders = elf32::readProgramHeaders(data, length, header, count);
    if(!programHeaders)
      return -1;

    auto& oldMemoryMapping = memory::MemoryMapping::current();
    task.memoryMapping()->makeCurrent();
    for(size_t i=0; i<count; ++i)
      if(loadProgramHeader(data, length, task, programHeaders[i]) != 0)
        return -1;

    task.asUserspaceTask(header->e_entry);
    oldMemoryMapping.makeCurrent();

    return 0;
  }
}
