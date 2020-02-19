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

extern "C" BOOT_FUNCTION void lower_half_main(std::byte* boot_information)
{
  // 1: Parse Boot Information
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
        to_physical(bootInformation.mmap) = mmap;

        size_t i = 0;
        for(struct multiboot_mmap_entry* entry = mmap.entries; 
            reinterpret_cast<std::byte*>(entry) < reinterpret_cast<std::byte*>(mmap.entries) + mmap.size;
            entry = reinterpret_cast<struct multiboot_mmap_entry*>(reinterpret_cast<std::byte*>(entry) + mmap.entry_size))
          to_physical(bootInformation.mmap_entries[i++]) = *entry;
        to_physical(bootInformation.mmap_entries_count) = i;
        break;
      }
      case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:      
      {
        auto& framebuffer = *reinterpret_cast<struct multiboot_tag_framebuffer*>(tag);
        to_physical(bootInformation.framebuffer) = framebuffer;
        break;
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
