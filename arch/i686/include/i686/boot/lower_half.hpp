#pragma once

#include <grub/multiboot2.h>

#include <cstddef>

#include <boot/boot.hpp>

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

extern "C"
{
  BOOT_FUNCTION [[gnu::noinline]] void lower_half_main(std::byte* tag);
}
