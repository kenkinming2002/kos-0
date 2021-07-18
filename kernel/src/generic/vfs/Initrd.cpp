#include <generic/vfs/Initrd.hpp>

#include <generic/vfs/VFS.hpp>
#include <generic/vfs/Tar.hpp>

#include <generic/memory/Memory.hpp>

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
        if(moduleEntry.cmdline == rt::StringRef("initrd"))
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
          auto result = createAt(rootVnode, tarHeaderBlock->fileName(), Type::REGULAR_FILE);
          if(!result)
            rt::panic("Failed to create file from initrd\n");

          auto file = openAt(rootVnode, tarHeaderBlock->fileName());
          if(!file)
            rt::panic("Failed to open file after creation\n");

          // TODO: Check if data is in range
          (*file)->resize(tarHeaderBlock->fileSize());
          (*file)->write(tarHeaderBlock->fileData(), tarHeaderBlock->fileSize());
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

    auto data   = reinterpret_cast<char*>(memory::physToVirt(initrdModule->addr));
    auto length = initrdModule->len;
    loadInitrd(data, length);
    memory::freePages(data, (length + memory::PAGE_SIZE - 1) / memory::PAGE_SIZE);
  }
}
