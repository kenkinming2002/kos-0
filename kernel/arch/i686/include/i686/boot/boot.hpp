#pragma once

#include <generic/utils/Utilities.hpp>

#include <generic/grub/multiboot2.h>
#include <stdint.h>
#include <cstddef>

#include <i686/core/Segmentation.hpp>

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


struct BootInformation
{
// Memory
public:
  struct MemoryMapEntry
  {
  public:
    BOOT_FUNCTION MemoryMapEntry(struct multiboot_mmap_entry* mmap_tag_entry);

  public:
    uintptr_t addr;
    size_t len;
    enum class Type { AVAILABLE, BOOT_INFORMATION, ACPI, RESERVED, DEFECTIVE } type;
  };
  MemoryMapEntry* memoryMapEntries;
  size_t memoryMapEntriesCount;

// TODO: Modules
public:
  struct ModuleEntry
  {
  public:
    void* addr;
    size_t len;

    ModuleEntry* next;
  };
  ModuleEntry* moduleEntries;

// Framebuffer
public:
  struct multiboot_tag_framebuffer frameBuffer;
};

// Output
extern std::byte bootInformationStorage[];



//  TODO: set this depending on kernel size
constexpr size_t BOOT_PAGE_TABLE_COUNT = 1;

extern std::byte kernelPageDirectory[];
extern std::byte kernelPageTable[];

// Have to work around a weird quark of gcc that section attributes is silently
// ignored for template function
template<typename T>
FORCE_INLINE BOOT_FUNCTION inline T& to_physical(T& t)
{
  return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(&t) - 0xC0000000);
}


