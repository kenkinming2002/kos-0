#include <boot/generic/BootInformation.hpp>

#include <boot/i686/Paging.hpp>
#include <boot/generic/Kernel.hpp>
#include <boot/generic/multiboot2-Utils.hpp>
#include <boot/generic/Config.h>
#include <boot/generic/Memory.hpp>

#include <librt/Panic.hpp>
#include <librt/Strings.hpp>

namespace boot
{
  void bootInformationInitialize(BootInformation& bootInformation, struct multiboot_boot_information* multiboot2BootInformation)
  {
    using namespace boot::memory;
    using namespace common::memory;

    for(auto* tag = multiboot2BootInformation->tags; tag->type != MULTIBOOT_TAG_TYPE_END; tag = multiboot2::next_tag(tag))
      switch(tag->type)
      {
        case MULTIBOOT_TAG_TYPE_MMAP:
          {
            // Somehow using constructor does not work here, but leaving
            // MemoryMapEntry as aggregate works. I have no idea why. Probably I am
            // invoking some form of UB.
            auto* mmapTag = reinterpret_cast<struct multiboot_tag_mmap*>(tag);
            for (auto mmapEntry = multiboot2::mmap_entry_begin(mmapTag); mmapEntry < multiboot2::mmap_entry_end(mmapTag); mmapEntry = multiboot2::next_mmap_entry(mmapTag, mmapEntry))
            {
              bootInformation.memoryMapEntries[bootInformation.memoryMapEntriesCount].addr = mmapEntry->addr;
              bootInformation.memoryMapEntries[bootInformation.memoryMapEntriesCount].len = mmapEntry->len;
              switch (mmapEntry->type)
              {
                case 1:  bootInformation.memoryMapEntries[bootInformation.memoryMapEntriesCount].type = MemoryMapEntry::Type::AVAILABLE; break;
                case 3:  bootInformation.memoryMapEntries[bootInformation.memoryMapEntriesCount].type = MemoryMapEntry::Type::ACPI;      break;
                case 4:  bootInformation.memoryMapEntries[bootInformation.memoryMapEntriesCount].type = MemoryMapEntry::Type::RESERVED;  break;
                case 5:  bootInformation.memoryMapEntries[bootInformation.memoryMapEntriesCount].type = MemoryMapEntry::Type::DEFECTIVE; break;
                default: bootInformation.memoryMapEntries[bootInformation.memoryMapEntriesCount].type = MemoryMapEntry::Type::RESERVED;  break;
              }
              ++bootInformation.memoryMapEntriesCount;
            }
            break;
          }
        case MULTIBOOT_TAG_TYPE_MODULE:
          {
            // Same problem here. Simply assigning result of aggregate
            // initialization does not work.
            auto* module_tag = reinterpret_cast<struct multiboot_tag_module*>(tag);
            auto& moduleEntry = bootInformation.moduleEntries[bootInformation.moduleEntriesCount];
            moduleEntry.addr = module_tag->mod_start;
            moduleEntry.len  = module_tag->mod_end-module_tag->mod_start;
            rt::strncpy(moduleEntry.cmdline, module_tag->cmdline, sizeof moduleEntry.cmdline);
            ++bootInformation.moduleEntriesCount;
            break;
          }
        case MULTIBOOT_TAG_TYPE_CMDLINE:
          {
            auto* string_tag = reinterpret_cast<struct multiboot_tag_string*>(tag);
            rt::strncpy(bootInformation.cmdline, string_tag->string, sizeof bootInformation.cmdline);
            break;
          }
      }

    uintptr_t result;

    result = memory::map(bootInformation, reinterpret_cast<uintptr_t>(memory::pageDirectory), PAGE_SIZE, Access::SUPERVISOR_ONLY, Permission::READ_WRITE);
    if(result == MAP_FAILED)
      rt::panic("Failed to map page directory");

    bootInformation.pageDirectory = reinterpret_cast<PageDirectory*>(result);
  }
}
