#include <i686/boot/lower_half.hpp>

#include <generic/utils/Utilities.hpp>

BootInformation bootInformation;

__attribute__((aligned(4096))) std::byte bootPageDirectory[sizeof(boot::PageDirectory)];
__attribute__((aligned(4096))) std::byte bootPageTable[sizeof(boot::PageTable) * BOOT_PAGE_TABLE_COUNT];

__attribute__((aligned(4096))) std::byte bootGDTEntries[sizeof(boot::GDTEntry) * GDT_SIZE];

// Have to work around a weird quark of gcc that section attributes is silently
// ignored for template function
template<typename T>
FORCE_INLINE BOOT_FUNCTION inline T& to_physical(T& t)
{
  return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(&t) - 0xC0000000);
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

  // 2: set up segmentation
  auto& GDTEntries = utils::deref_cast<boot::GDTEntry[GDT_SIZE]>(to_physical(bootGDTEntries));
  // None segment in the beginning
  GDTEntries[0] = boot::GDTEntry(0, 0,          boot::PrivillegeLevel::RING0, boot::SegmentType::NONE_SEGMENT);

  // Kernel segments
  GDTEntries[1] = boot::GDTEntry(0, 0xffffffff, boot::PrivillegeLevel::RING0, boot::SegmentType::CODE_SEGMENT_RD);
  GDTEntries[2] = boot::GDTEntry(0, 0xffffffff, boot::PrivillegeLevel::RING0, boot::SegmentType::DATA_SEGMENT);

  GDTEntries[3] = boot::GDTEntry(0, 0xffffffff, boot::PrivillegeLevel::RING3, boot::SegmentType::CODE_SEGMENT_RD);
  GDTEntries[4] = boot::GDTEntry(0, 0xffffffff, boot::PrivillegeLevel::RING3, boot::SegmentType::DATA_SEGMENT);

  boot::GDT(GDTEntries, GDT_SIZE).load();

  // 3: Setup Paging
  auto& pageDirectory = utils::deref_cast<boot::PageDirectory>(to_physical(bootPageDirectory));
  auto& pageTables    = utils::deref_cast<boot::PageTable[BOOT_PAGE_TABLE_COUNT]>(to_physical(bootPageTable));
  for(size_t pageDirectoryIndex=0; pageDirectoryIndex<BOOT_PAGE_TABLE_COUNT; ++pageDirectoryIndex)
  {
    auto& pageTable = pageTables[pageDirectoryIndex];
    for(size_t pageTableIndex=0; pageTableIndex<boot::PAGE_TABLE_ENTRY_COUNT; ++pageTableIndex)
    {
      auto& pageTableEntry = pageTable[pageTableIndex];
      auto pageTablesIndex = pageDirectoryIndex * 1024u + pageTableIndex;
      pageTableEntry = boot::PageTableEntry(pageTablesIndex * 4096u, boot::TLBMode::LOCAL, boot::CacheMode::ENABLED, boot::WriteMode::WRITE_BACK, boot::Access::SUPERVISOR_ONLY, boot::Permission::READ_WRITE);
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
