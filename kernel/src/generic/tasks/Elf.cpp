#include <common/generic/tasks/Elf.hpp>
#include <common/i686/memory/Paging.hpp>
#include "generic/memory/Pages.hpp"
#include "i686/memory/MemoryMapping.hpp"
#include <generic/tasks/Elf.hpp>

#include <limits>
#include <optional>
#include <algorithm>

namespace core::tasks
{
  using namespace common::tasks;

  static bool checkDataRange(const char* data, size_t length, Elf32_Off off, Elf32_Word size)
  {
    Elf32_Addr result;
    if(__builtin_add_overflow(off, size, &result))
      return false;
    return result<=length;
  }

  static bool checkVirtualRange(Elf32_Off off, Elf32_Word size)
  {
    Elf32_Addr result;
    if(__builtin_add_overflow(off, size, &result))
      return false;
    return result<=0xC0000000;
  }

  static int loadProgramHeader(const char* data, size_t length, Task& task, const ELF32ProgramHeader& programHeader)
  {
    using namespace core::memory;
    using namespace common::memory;

    if(!checkDataRange(data, length, programHeader.offset, programHeader.filesz))
      return -1;

    if(!checkVirtualRange(programHeader.vaddr, programHeader.memsz))
      return -1;

    if(programHeader.filesz>programHeader.memsz)
      return -1; // It is better to be more pedantic in OS code

    // TODO: Gracefully handle any error
    if(!(programHeader.flags & PF_R))
      return -1; // We do not support a page that is not readable
    auto pagesPermission = (programHeader.flags & PF_W) ? Permission::READ_WRITE : Permission::READ_ONLY;
    task.memoryMapping().map(Pages::fromAggressive(programHeader.vaddr, programHeader.vaddr+programHeader.memsz), Access::ALL, pagesPermission);

    auto fileSegmentBegin    = reinterpret_cast<const char*>(data+programHeader.offset);
    auto fileSegmentEnd      = reinterpret_cast<const char*>(data+programHeader.offset+programHeader.filesz);
    auto virtualSegmentBegin = reinterpret_cast<char*>(programHeader.vaddr);
    auto virtualSegmentEnd   = reinterpret_cast<char*>(programHeader.vaddr+programHeader.memsz);

    std::fill(virtualSegmentBegin, virtualSegmentEnd, '\0');
    std::copy(fileSegmentBegin, fileSegmentEnd, virtualSegmentBegin);

    return 0;
  }

  std::optional<Task> loadElf(char* data, size_t length)
  {
    if(length<sizeof(Elf32Header))
      return std::nullopt;

    const auto* header = getElf32Header(data, length);
    if(!header)
      return std::nullopt;

    size_t count;
    const auto* programHeaders = getElf32ProgramHeaders(data, length, header, count);
    if(!programHeaders)
      return std::nullopt;

    auto task = Task::allocate();
    if(!task)
      return std::nullopt;

    auto& oldMemoryMapping = memory::MemoryMapping::current();
    task->memoryMapping().makeCurrent();
    for(size_t i=0; i<count; ++i)
      if(loadProgramHeader(data, length, *task, programHeaders[i]) != 0)
        return std::nullopt;
    task->asUserspaceTask(header->entry);
    oldMemoryMapping.makeCurrent();

    return task;
  }
}
