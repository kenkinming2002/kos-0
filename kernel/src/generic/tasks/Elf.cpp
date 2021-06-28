#include "i686/tasks/Task.hpp"
#include "librt/SharedPtr.hpp"
#include <generic/tasks/Elf.hpp>

#include <common/i686/memory/Paging.hpp>

#include <i686/syscalls/Access.hpp>
#include <i686/memory/MemoryMapping.hpp>

#include <libelf/libelf.hpp>

#include <librt/Optional.hpp>
#include <librt/Algorithm.hpp>

namespace core::tasks
{
  namespace
  {
    Result<void> loadProgramHeader(rt::SharedPtr<Task> task, rt::SharedPtr<vfs::File> file, const Elf32_Phdr& programHeader)
    {
      using namespace core::memory;
      using namespace common::memory;

      if(programHeader.p_type == PT_LOAD)
      {
        if(!syscalls::verifyRegionUser(programHeader.p_vaddr, programHeader.p_memsz))
          return ErrorCode::INVALID;

        if(programHeader.p_filesz>programHeader.p_memsz)
          return ErrorCode::INVALID; // It is better to be more pedantic in OS code

        // TODO: Gracefully handle any error
        if(!(programHeader.p_flags & PF_R))
          return ErrorCode::INVALID; // We do not support a page that is not readable

        auto pagesPermission = (programHeader.p_flags & PF_W) ? Permission::READ_WRITE : Permission::READ_ONLY;
        task->memoryMapping()->map(programHeader.p_vaddr, programHeader.p_memsz, pagesPermission, file, programHeader.p_offset);
      }

      return {};
    }
  }

  Result<void> loadElf(rt::SharedPtr<Task> task, rt::SharedPtr<vfs::File> file)
  {
    Elf32_Ehdr header;
    file->seek(Anchor::BEGIN, 0);
    file->read(reinterpret_cast<char*>(&header), sizeof header);

    if(header.e_ident[EI_MAG0] != 0x7f || header.e_ident[EI_MAG1] != 'E'  || header.e_ident[EI_MAG2] != 'L'  || header.e_ident[EI_MAG3] != 'F')
      return ErrorCode::INVALID;

    if(header.e_ident[EI_CLASS] != ELFCLASS32)
      return ErrorCode::INVALID;

    if(header.e_ident[EI_DATA] != ELFDATA2LSB)
      return ErrorCode::INVALID;

    if(header.e_ident[EI_VERSION] != EV_CURRENT)
      return ErrorCode::INVALID;

    auto previousTask = tasks::Task::current();
    tasks::Task::makeCurrent(task);

    // FIXME: Set an upper limit
    Elf32_Phdr programHeaders[header.e_phnum];
    file->seek(Anchor::BEGIN, header.e_phoff);
    file->read(reinterpret_cast<char*>(programHeaders), sizeof programHeaders);
    for(size_t i=0; i<header.e_phnum; ++i)
      if(auto result = loadProgramHeader(task, file, programHeaders[i]); !result)
        return result.error();

    task->asUserspaceTask(header.e_entry);

    tasks::Task::makeCurrent(rt::move(previousTask));
    return {};
  }
}
