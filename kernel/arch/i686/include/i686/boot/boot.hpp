#pragma once

#include <generic/utils/Utilities.hpp>

#include <generic/grub/multiboot2.h>
#include <stdint.h>
#include <cstddef>

/**
 * @file include/i686/boot/boot.hpp
 *
 * Basics macro for accessing data and functions at boot time before higher-half
 * kernel is set-up.
 */

#define BOOT_FUNCTION [[gnu::section(".boot.text")]]

// Obtain address of data before setting up of paging (i.e. convert virtual
// address to physical address)
#define BOOT_ADDRESS

// Have to work around a weird quark of gcc that section attributes is silently
// ignored for template function
template<typename T>
FORCE_INLINE BOOT_FUNCTION inline T& to_physical(T& t)
{
  return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(&t) - 0xC0000000);
}


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

extern std::byte kernelPageDirectory[];
extern std::byte kernelPageTable[];

constexpr static size_t GDT_SIZE = 5;
extern std::byte kernelGDTEntries[];
