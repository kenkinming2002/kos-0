#include <boot/i686/LowerHalf.hpp>

#include <boot/i686/BootInformation.hpp>
#include <common/i686/memory/Paging.hpp>

#include <stdint.h>
#include <stddef.h>


extern "C"
{
  extern char kernel_boot_section_virtual_begin [];
  extern char kernel_boot_section_virtual_end [];

  extern char kernel_read_only_section_begin [];
  extern char kernel_read_only_section_end [];

  extern char kernel_read_write_section_begin [];
  extern char kernel_read_write_section_end [];
}

namespace
{
  template<typename T>
  [[gnu::always_inline]] inline T* byte_advance(T* t, size_t n)
  {
    return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(t)+n);
  }

  template<typename T>
  [[gnu::always_inline]] inline T* physical_address(T* t) { return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(t)-0xC0000000); }

  template<typename T>
  [[gnu::always_inline]] inline T* virtual_address(T* t) { return reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(t)+0xC0000000); }
  [[gnu::always_inline]] inline uintptr_t virtual_address(uintptr_t t) { return t+0xC0000000; }

  [[gnu::always_inline]] inline uintptr_t align_up(uintptr_t ptr, size_t alignment)
  {
    return (ptr + (alignment - 1)) & ~(alignment-1);
  }

  [[gnu::always_inline]] inline char* align_up(char* ptr, size_t alignment)
  {
    return reinterpret_cast<char*>(align_up(reinterpret_cast<uintptr_t>(ptr), alignment));
  }

  class Allocator
  {
  public:
    [[gnu::always_inline]] Allocator() : buffer(physical_address(bootInformationStorageBegin)) { }

  public:
    [[gnu::always_inline]] char* allocate(size_t size, size_t alignment)
    {
      char* result = align_up(buffer, alignment);
      buffer = result + size;
      return result;
    }

    template<typename T>
    [[gnu::always_inline]] T* allocate(size_t count=1)
    {
      return reinterpret_cast<T*>(allocate(sizeof(T) * count, alignof(T)));
    }

  private:
    char* buffer;
  };
}

namespace
{
  BOOT_FUNCTION void parse_boot_information(char* boot_information)
  {
    Allocator allocator;

    auto* bootInformation = allocator.allocate<BootInformation>();

    bootInformation->memoryMapEntriesCount = 0;
    bootInformation->moduleEntriesCount = 0;

    for(struct multiboot_tag* tag = reinterpret_cast<struct multiboot_tag*>(boot_information + 8);
      tag->type != MULTIBOOT_TAG_TYPE_END;
      tag = reinterpret_cast<struct multiboot_tag *>(reinterpret_cast<char*>(tag) + ((tag->size+7) & ~7))) 
    {
      switch(tag->type)
      {
      case MULTIBOOT_TAG_TYPE_MMAP:             
      {
        auto& mmap_tag = *reinterpret_cast<struct multiboot_tag_mmap*>(tag);
        for(auto *entry = mmap_tag.entries, *end = byte_advance(mmap_tag.entries, mmap_tag.size - sizeof mmap_tag); entry < end; entry = byte_advance(entry, mmap_tag.entry_size)) 
          ++bootInformation->memoryMapEntriesCount;
        break;
      }
      case MULTIBOOT_TAG_TYPE_MODULE:
        ++bootInformation->moduleEntriesCount;
        break;
      }
    }

    bootInformation->memoryMapEntries = allocator.allocate<MemoryMapEntry>(bootInformation->memoryMapEntriesCount);
    bootInformation->moduleEntries    = allocator.allocate<ModuleEntry>(bootInformation->moduleEntriesCount);

    size_t memoryMapEntriesIndex = 0, moduleEntriesIndex=0;
    for(struct multiboot_tag* tag = reinterpret_cast<struct multiboot_tag*>(boot_information + 8);
      tag->type != MULTIBOOT_TAG_TYPE_END;
      tag = reinterpret_cast<struct multiboot_tag *>(reinterpret_cast<char*>(tag) + ((tag->size+7) & ~7))) 
    {
      switch(tag->type)
      {
      case MULTIBOOT_TAG_TYPE_MMAP:             
      {
        // Somehow using constructor does not work here, but leaving
        // MemoryMapEntry as aggregate works. I have no idea why. Probably I am
        // invoking some form of UB.
        auto& mmap_tag = *reinterpret_cast<struct multiboot_tag_mmap*>(tag);
        for(auto *entry = mmap_tag.entries, *end = byte_advance(mmap_tag.entries, mmap_tag.size - sizeof mmap_tag); entry < end; entry = byte_advance(entry, mmap_tag.entry_size)) 
        {
          bootInformation->memoryMapEntries[memoryMapEntriesIndex].addr = entry->addr;
          bootInformation->memoryMapEntries[memoryMapEntriesIndex].len  = entry->len;
          switch(entry->type)
          {
            case 1:  bootInformation->memoryMapEntries[memoryMapEntriesIndex].type = MemoryMapEntry::Type::AVAILABLE; break;
            case 3:  bootInformation->memoryMapEntries[memoryMapEntriesIndex].type = MemoryMapEntry::Type::ACPI;      break;
            case 4:  bootInformation->memoryMapEntries[memoryMapEntriesIndex].type = MemoryMapEntry::Type::RESERVED;  break;
            case 5:  bootInformation->memoryMapEntries[memoryMapEntriesIndex].type = MemoryMapEntry::Type::DEFECTIVE; break;
            default: bootInformation->memoryMapEntries[memoryMapEntriesIndex].type = MemoryMapEntry::Type::RESERVED;  break;
          }
          ++memoryMapEntriesIndex;
        }
        break;
      }
      case MULTIBOOT_TAG_TYPE_MODULE:
      {
        // Same problem here. Simply assigning result of aggregate
        // initialization does not work.
        auto* module_tag = reinterpret_cast<struct multiboot_tag_module*>(tag);
        bootInformation->moduleEntries[moduleEntriesIndex].addr = module_tag->mod_start; 
        bootInformation->moduleEntries[moduleEntriesIndex].len  = module_tag->mod_end-module_tag->mod_start;
        ++moduleEntriesIndex;
        break;
      }
      case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
        bootInformation->frameBuffer = *reinterpret_cast<struct multiboot_tag_framebuffer*>(tag);
        break;
      }
    }


    bootInformation->memoryMapEntries = virtual_address(bootInformation->memoryMapEntries);
    bootInformation->moduleEntries    = virtual_address(bootInformation->moduleEntries);
    bootInformation->frameBuffer.common.framebuffer_addr = virtual_address(bootInformation->frameBuffer.common.framebuffer_addr);
  }

  BOOT_FUNCTION void setup_paging()
  {
    auto& pageDirectory = *physical_address(&initialPageDirectory);
    auto& pageTables    = *physical_address(&kernelPageTables);

    for(size_t pageDirectoryIndex=0; pageDirectoryIndex<BOOT_PAGE_TABLE_COUNT; ++pageDirectoryIndex)
    {
      auto& pageTable = pageTables[pageDirectoryIndex];
      for(size_t pageTableIndex=0; pageTableIndex<common::memory::PAGE_TABLE_ENTRY_COUNT; ++pageTableIndex)
      {
        auto& pageTableEntry = pageTable[pageTableIndex];
        uintptr_t physicalAddress = (pageDirectoryIndex * 1024u + pageTableIndex) * 4096u;
        uintptr_t virtualAddress = physicalAddress + 0xC0000000;

        // Video Memory & Rom Area => Map it Read/Write anyway since it maps to
        //                            VGA(if it exists) & possibly some mapped
        //                            hardware
        if(physicalAddress < 0x00100000)
        {
          pageTableEntry = common::memory::PageTableEntry(physicalAddress, common::memory::TLBMode::LOCAL, common::memory::CacheMode::ENABLED, common::memory::WriteMode::WRITE_BACK, common::memory::Access::SUPERVISOR_ONLY, common::memory::Permission::READ_WRITE);
          continue;
        }
        if(virtualAddress>=reinterpret_cast<uintptr_t>(kernel_boot_section_virtual_begin) && virtualAddress < reinterpret_cast<uintptr_t>(kernel_boot_section_virtual_end))
        {
          pageTableEntry = common::memory::PageTableEntry(physicalAddress, common::memory::TLBMode::LOCAL, common::memory::CacheMode::ENABLED, common::memory::WriteMode::WRITE_BACK, common::memory::Access::SUPERVISOR_ONLY, common::memory::Permission::READ_ONLY);
          continue;
        }
        if(virtualAddress>=reinterpret_cast<uintptr_t>(kernel_read_only_section_begin) && virtualAddress < reinterpret_cast<uintptr_t>(kernel_read_only_section_end))
        {
          pageTableEntry = common::memory::PageTableEntry(physicalAddress, common::memory::TLBMode::LOCAL, common::memory::CacheMode::ENABLED, common::memory::WriteMode::WRITE_BACK, common::memory::Access::SUPERVISOR_ONLY, common::memory::Permission::READ_ONLY);
          continue;
        }
        if(virtualAddress>=reinterpret_cast<uintptr_t>(kernel_read_only_section_begin) && virtualAddress < reinterpret_cast<uintptr_t>(kernel_read_only_section_end))
        {
          pageTableEntry = common::memory::PageTableEntry(physicalAddress, common::memory::TLBMode::LOCAL, common::memory::CacheMode::ENABLED, common::memory::WriteMode::WRITE_BACK, common::memory::Access::SUPERVISOR_ONLY, common::memory::Permission::READ_ONLY);
          continue;
        }
        if(virtualAddress>=reinterpret_cast<uintptr_t>(kernel_read_write_section_begin) && virtualAddress < reinterpret_cast<uintptr_t>(kernel_read_write_section_end))
        {
          pageTableEntry = common::memory::PageTableEntry(physicalAddress, common::memory::TLBMode::LOCAL, common::memory::CacheMode::ENABLED, common::memory::WriteMode::WRITE_BACK, common::memory::Access::SUPERVISOR_ONLY, common::memory::Permission::READ_WRITE);
          continue;
        }
        // We cannot disable it just now as it contains code in our boot section.
        // We can do so as soon as we enter higher half
        //pageTableEntry = common::memory::PageTableEntry(physicalAddress, common::memory::TLBMode::LOCAL, common::memory::CacheMode::ENABLED, common::memory::WriteMode::WRITE_BACK, common::memory::Access::SUPERVISOR_ONLY, common::memory::Permission::READ_ONLY);
      }

      auto& lowerHalfPageDirectoryEntry  = pageDirectory[pageDirectoryIndex];
      lowerHalfPageDirectoryEntry  = common::memory::PageDirectoryEntry(reinterpret_cast<uintptr_t>(&pageTable), common::memory::CacheMode::ENABLED, common::memory::WriteMode::WRITE_BACK, common::memory::Access::SUPERVISOR_ONLY, common::memory::Permission::READ_WRITE);
      auto& higherHalfPageDirectoryEntry = pageDirectory[pageDirectoryIndex + 768];
      higherHalfPageDirectoryEntry = common::memory::PageDirectoryEntry(reinterpret_cast<uintptr_t>(&pageTable), common::memory::CacheMode::ENABLED, common::memory::WriteMode::WRITE_BACK, common::memory::Access::SUPERVISOR_ONLY, common::memory::Permission::READ_WRITE);
    }

    asm volatile ( 
      // Load page directory
      "mov cr3, %[pageDirectory];"

      // Set PSE(Page Size Extension) bit
      "mov eax, cr4;"       
      "or  eax, 0x00000010;"
      "mov cr4, eax;"

      // Set PG(Paging) and PE(Protention) bit
      "mov eax, cr0;"         
      "or  eax, 0x80000001;"  
      "mov cr0, eax;"
     : : [pageDirectory]"r"(&pageDirectory) : "eax", "memory");
  }
}

extern "C" BOOT_FUNCTION void lower_half_main(char* boot_information)
{
  parse_boot_information(boot_information);
  setup_paging();
}
