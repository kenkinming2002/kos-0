#include <generic/vfs/Initrd.hpp>

#include <generic/vfs/VFS.hpp>
#include <generic/vfs/Tar.hpp>

#include <generic/memory/Memory.hpp>
#include <generic/memory/Pages.hpp>

#include <generic/BootInformation.hpp>

#include <librt/Strings.hpp>
#include <librt/Algorithm.hpp>
#include <librt/StringRef.hpp>
#include <librt/Panic.hpp>

namespace core::vfs
{
  namespace
  {
    const ModuleEntry* findInitrdModule()
    {
      for(size_t i=0; i<bootInformation->moduleEntriesCount; ++i)
      {
        auto& moduleEntry = bootInformation->moduleEntries[i];
        if(moduleEntry.cmdline == rt::StringRef("initrd", 6))
          return &moduleEntry;
      }

      return nullptr;
    }

    void loadInitrd(const char* initrd, size_t length)
    {
      // The Initrd is in an archive format, specifically, it is a tar archive.
      auto rootVnode = root();
      for(auto* tarHeaderBlock = reinterpret_cast<const TarFileBlock*>(initrd);
          !tarHeaderBlock->isNullHeader(); // TODO: Check if tar header block is in range
          tarHeaderBlock = tarHeaderBlock->nextHeaderBlock())
      {
        if(!tarHeaderBlock->isValidHeader())
          rt::panic("Corrupted header in tar formatted initramfs\n");

        if(tarHeaderBlock->isRegularFile())
        {
          auto file = createAt(rootVnode, tarHeaderBlock->fileName(), Type::REGULAR_FILE);
          if(!file)
            rt::panic("Failed to create file from initrd\n");

          // TODO: Check if data is in range
          file->resize(tarHeaderBlock->fileSize());
          file->write(tarHeaderBlock->fileData(), tarHeaderBlock->fileSize(), 0);
        }
        else if(tarHeaderBlock->isDirectory())
        {
          auto file = createAt(rootVnode, tarHeaderBlock->fileName(), Type::DIRECTORY);
          if(!file)
            rt::panic("Failed to create directory from initrd\n");
        }
        else
          rt::log("Unknown header block type detected\n");
      }
    }
  }

  void loadInitrd()
  {
    auto* initrdModule = findInitrdModule();
    if(!initrdModule)
      rt::panic("Failed to find initrd\n");

    auto pages = memory::mapPages(core::memory::Pages::fromAggressive(initrdModule->addr, initrdModule->len));
    if(!pages)
      rt::panic("Failed to map initrd\n");

    loadInitrd(reinterpret_cast<const char*>(pages->address()), pages->length());
    memory::freeMappedPages(*pages);
  }
}
