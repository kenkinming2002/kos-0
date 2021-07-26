#pragma once

#include <boot/generic/api/multiboot2.h>

#include <common/i686/memory/Paging.hpp>

#include <stdint.h>
#include <stddef.h>

static constexpr size_t MAX_MEMORY_MAP_ENTRIES_COUNT = 0x10;
static constexpr size_t MAX_MODULE_ENTRIES_COUNT     = 0x10;
static constexpr size_t MAX_MEMORY_REGIONS_COUNT     = 0x10;
static constexpr size_t MAX_CMDLINE_LENGTH           = 0x20; // 32 Characters

struct MemoryMapEntry
{
public:
  uintptr_t addr;
  size_t len;
  enum class Type { AVAILABLE, ACPI, RESERVED, DEFECTIVE } type;
};

struct ModuleEntry
{
public:
  uintptr_t addr;
  size_t len;
  char cmdline[MAX_CMDLINE_LENGTH];
};

struct ReservedMemoryRegion
{
  uintptr_t addr;
  size_t len;
  enum class Type { KERNEL, PAGING, BOOT_INFORMATION, MISC } type;
};

// I need a way to tell the kernel what memory regions it should not use
// until it is done with it.
//
// One example is PageDirectory and PageTables used.
// Another example is physical memory used to load the kernel.
// Modules is also one such example.
// Memory is also needed to store BootInformation structure.
//
// At the end, only memory used for kernel is not reclaimed.
//
// TODO: Change it so that we can expand entries count without breaking
//       binary compatibility
struct BootInformation
{
  uint8_t coresCount; // This must be the first element

  uintptr_t physicalMemoryOffset;
  size_t physicalMemoryLength;

  size_t memoryMapEntriesCount;
  size_t moduleEntriesCount;
  size_t reservedMemoryRegionsCount;

  char cmdline[MAX_CMDLINE_LENGTH];
  MemoryMapEntry memoryMapEntries[MAX_MEMORY_MAP_ENTRIES_COUNT];
  ModuleEntry moduleEntries[MAX_MODULE_ENTRIES_COUNT];
  ReservedMemoryRegion reservedMemoryRegions[MAX_MEMORY_REGIONS_COUNT];
};

