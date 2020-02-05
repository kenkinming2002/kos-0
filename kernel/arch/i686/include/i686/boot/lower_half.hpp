#pragma once

#include <grub/multiboot2.h>

#include <cstddef>

#include <boot/boot.hpp>
#include <i686/boot/Segmentation.hpp>
#include <i686/boot/Paging.hpp>

struct BootInformation
{
public:
  constexpr static size_t MAX_MMAP_ENTRIES_COUNT = 16;

public:
  struct multiboot_tag_framebuffer framebuffer;
  struct multiboot_tag_mmap        mmap;
  struct multiboot_mmap_entry      mmap_entries[MAX_MMAP_ENTRIES_COUNT];
  size_t                           mmap_entries_count;
};

// Output
extern BootInformation bootInformation;

//  TODO: set this depending on kernel size
constexpr size_t BOOT_PAGE_TABLE_COUNT = 1;
extern std::byte bootPageDirectory[sizeof(boot::PageDirectory)];
extern std::byte bootPageTable[sizeof(boot::PageTable) * BOOT_PAGE_TABLE_COUNT];

constexpr static size_t GDT_SIZE = 5;
extern std::byte bootGDTEntries[sizeof(boot::GDTEntry) * GDT_SIZE];

extern "C" BOOT_FUNCTION [[gnu::noinline]] void lower_half_main(std::byte* tag);
