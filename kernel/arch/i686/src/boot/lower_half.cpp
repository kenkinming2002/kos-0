#include <i686/boot/lower_half.hpp>

#include <i686/boot/Segmentation.hpp>
#include <i686/boot/Paging.hpp>

#include <generic/utils/Utilities.hpp>

#include <stdint.h>

extern "C"
{
  extern std::byte kernel_read_only_section_begin [];
  extern std::byte kernel_read_only_section_end [];

  extern std::byte kernel_read_write_section_begin [];
  extern std::byte kernel_read_write_section_end [];
}

std::byte bootInformationStorage[4096u];

BOOT_FUNCTION BootInformation::MemoryMapEntry::MemoryMapEntry(struct multiboot_mmap_entry* mmap_tag_entry)
  : addr(mmap_tag_entry->addr), len(mmap_tag_entry->len) 
{
  type = [&](){
    switch(mmap_tag_entry->type)
    {
    case 1:
      return Type::AVAILABLE;
    case 3:
      return Type::ACPI;
    case 4:
      return Type::RESERVED;
    case 5:
      return Type::DEFECTIVE;
    default:
      return Type::RESERVED;
    }
  }();
}

namespace
{
  // NOTE: consider adding buffer size check
  BOOT_FUNCTION void align(void*& buf, std::size_t& capacity, std::size_t alignment)
  {
    // Aligned the buf pointer
    void* aligned =  reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(buf) + alignment - 1) / alignment * alignment);
    size_t padding = reinterpret_cast<uintptr_t>(aligned)-reinterpret_cast<uintptr_t>(buf);

    if(capacity<padding)
      for(;;) asm("hlt");
    capacity -=  padding;
    buf = aligned;
  }

  BOOT_FUNCTION void* alloc(void*& buf, std::size_t& capacity, std::size_t size)
  {
    if(capacity<size)
      for(;;) asm("hlt");

    // Increment the buf pointer
    void* allocResult = buf;

    buf = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(buf) + size);
    capacity-=size;

    return allocResult;
  }
}

extern "C" BOOT_FUNCTION void lower_half_main(std::byte* boot_information)
{
  // 1: Parse Boot Information
  {
    void* bootInformationBuf = static_cast<void*>(to_physical(bootInformationStorage));
    size_t bootInformationBufCapacity = 4096u;

    // 1: Allocate Memory for BootInformation
    align(bootInformationBuf, bootInformationBufCapacity, alignof(BootInformation));
    auto& bootInformation = *static_cast<BootInformation*>(alloc(bootInformationBuf, bootInformationBufCapacity, sizeof(BootInformation)));
    bootInformation.memoryMapEntries = nullptr;
    bootInformation.memoryMapEntriesCount = 0;
    bootInformation.moduleEntries = nullptr;

    // 2: Loop over Multiboot-Tag
    for(struct multiboot_tag* tag = reinterpret_cast<struct multiboot_tag*>(boot_information + 8);
        tag->type != MULTIBOOT_TAG_TYPE_END;
        tag = reinterpret_cast<struct multiboot_tag *>(reinterpret_cast<std::byte*>(tag) + ((tag->size+7) & ~7))
       )
    {
      switch(tag->type)
      {
        case MULTIBOOT_TAG_TYPE_MMAP:             
        {
          auto& mmap = *reinterpret_cast<struct multiboot_tag_mmap*>(tag);

          // Allocate Memory to store MemoryMapEntry but without specifying the
          // size since we do not know yet
          align(bootInformationBuf, bootInformationBufCapacity, alignof(BootInformation::MemoryMapEntry));
          auto& memoryMapEntries = *static_cast<BootInformation::MemoryMapEntry(*)[]>(bootInformationBuf);

          bootInformation.memoryMapEntriesCount = 0;
          for(struct multiboot_mmap_entry* entry = mmap.entries; 
              reinterpret_cast<std::byte*>(entry) < reinterpret_cast<std::byte*>(mmap.entries) + mmap.size;
              entry = reinterpret_cast<struct multiboot_mmap_entry*>(reinterpret_cast<std::byte*>(entry) + mmap.entry_size))
            memoryMapEntries[bootInformation.memoryMapEntriesCount++] = entry;

          // Now we know the size, commit it
          alloc(bootInformationBuf, bootInformationBufCapacity, bootInformation.memoryMapEntriesCount * sizeof(BootInformation::MemoryMapEntry));

          // Convert back to virtual address
          bootInformation.memoryMapEntries = reinterpret_cast<BootInformation::MemoryMapEntry*>(reinterpret_cast<uintptr_t>(&memoryMapEntries) + 0xC0000000);

          break;
        }
        case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:      
        {
          auto& framebuffer = *reinterpret_cast<struct multiboot_tag_framebuffer*>(tag);
          bootInformation.frameBuffer = framebuffer;
          break;
        }
        case MULTIBOOT_TAG_TYPE_MODULE:
        {
          auto* module_tag = reinterpret_cast<struct multiboot_tag_module*>(tag);

          // Allocate memory for Module
          size_t moduleSize = module_tag->mod_end - module_tag->mod_start;
          std::byte* module = static_cast<std::byte*>(alloc(bootInformationBuf, bootInformationBufCapacity, moduleSize));

          // Copy to Module
          for(size_t i=0; i<moduleSize; ++i)
            module[i] = reinterpret_cast<std::byte*>(module_tag->mod_start)[i];

          // Allocate memory for ModuleEntry
          align(bootInformationBuf, bootInformationBufCapacity, alignof(BootInformation::ModuleEntry));
          auto& moduleEntry = *static_cast<BootInformation::ModuleEntry*>(alloc(bootInformationBuf, bootInformationBufCapacity, sizeof(BootInformation::ModuleEntry)));
          moduleEntry.addr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(module) + 0xC0000000); // Convert to virtual address
          moduleEntry.len = moduleSize;
          
          // Append
          moduleEntry.next = bootInformation.moduleEntries;
          bootInformation.moduleEntries = reinterpret_cast<BootInformation::ModuleEntry*>(reinterpret_cast<uintptr_t>(&moduleEntry) + 0xC0000000);
          break;
        }
      }
    }
  }

  // 2: Setup Paging
  auto& pageDirectory = utils::deref_cast<boot::PageDirectory>(to_physical(kernelPageDirectory));
  auto& pageTables    = utils::deref_cast<boot::PageTable[BOOT_PAGE_TABLE_COUNT]>(to_physical(kernelPageTable));
  for(size_t pageDirectoryIndex=0; pageDirectoryIndex<BOOT_PAGE_TABLE_COUNT; ++pageDirectoryIndex)
  {
    auto& pageTable = pageTables[pageDirectoryIndex];
    for(size_t pageTableIndex=0; pageTableIndex<boot::PAGE_TABLE_ENTRY_COUNT; ++pageTableIndex)
    {
      auto& pageTableEntry = pageTable[pageTableIndex];
      uintptr_t physicalAddress = (pageDirectoryIndex * 1024u + pageTableIndex) * 4096u;
      uintptr_t virtualAddress = physicalAddress + 0xC0000000;

      // Video Memory & Rom Area => Map it Read/Write anyway since it maps to
      //                            VGA(if it exists) & possibly some mapped
      //                            hardware
      if(physicalAddress < 0x00100000)
      {
        pageTableEntry = boot::PageTableEntry(physicalAddress, boot::TLBMode::LOCAL, boot::CacheMode::ENABLED, boot::WriteMode::WRITE_BACK, boot::Access::SUPERVISOR_ONLY, boot::Permission::READ_WRITE);
        continue;
      }
      if(virtualAddress>=reinterpret_cast<uintptr_t>(kernel_read_only_section_begin) && virtualAddress < reinterpret_cast<uintptr_t>(kernel_read_only_section_end))
      {
        pageTableEntry = boot::PageTableEntry(physicalAddress, boot::TLBMode::LOCAL, boot::CacheMode::ENABLED, boot::WriteMode::WRITE_BACK, boot::Access::SUPERVISOR_ONLY, boot::Permission::READ_ONLY);
        continue;
      }
      if(virtualAddress>=reinterpret_cast<uintptr_t>(kernel_read_write_section_begin) && virtualAddress < reinterpret_cast<uintptr_t>(kernel_read_write_section_end))
      {
        pageTableEntry = boot::PageTableEntry(physicalAddress, boot::TLBMode::LOCAL, boot::CacheMode::ENABLED, boot::WriteMode::WRITE_BACK, boot::Access::SUPERVISOR_ONLY, boot::Permission::READ_WRITE);
        continue;
      }
      // We cannot disable it just now as it contains code in our boot section.
      // We can do so as soon as we enter higher half
      pageTableEntry = boot::PageTableEntry(physicalAddress, boot::TLBMode::LOCAL, boot::CacheMode::ENABLED, boot::WriteMode::WRITE_BACK, boot::Access::SUPERVISOR_ONLY, boot::Permission::READ_ONLY);
    }

    auto& lowerHalfPageDirectoryEntry  = pageDirectory[pageDirectoryIndex];
    lowerHalfPageDirectoryEntry  = boot::PageDirectoryEntry(reinterpret_cast<uintptr_t>(&pageTable), boot::CacheMode::ENABLED, boot::WriteMode::WRITE_BACK, boot::Access::SUPERVISOR_ONLY, boot::Permission::READ_WRITE);
    auto& higherHalfPageDirectoryEntry = pageDirectory[pageDirectoryIndex + 768];
    higherHalfPageDirectoryEntry = boot::PageDirectoryEntry(reinterpret_cast<uintptr_t>(&pageTable), boot::CacheMode::ENABLED, boot::WriteMode::WRITE_BACK, boot::Access::SUPERVISOR_ONLY, boot::Permission::READ_WRITE);
  }

  asm volatile ( R"(
    .intel_syntax noprefix
      mov cr3, %[pageDirectory]

      mov eax, cr4        # read cr4
      or  eax, 0x00000010 # set PSE bit
      mov cr4, eax        #

      mov eax, cr0        # read cr4
      or  eax, 0x80000001 # set PG bit
      mov cr0, eax        #

    .att_syntax prefix
    )"
    :
    : [pageDirectory]"r"(&pageDirectory)
    : "eax"
  );
}
