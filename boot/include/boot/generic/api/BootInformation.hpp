#pragma once

#include <boot/generic/api/multiboot2.h>
#include <boot/generic/api/Config.hpp>

#include <common/i686/memory/Paging.hpp>

#include <stdint.h>
#include <stddef.h>

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
  enum class Type { KERNEL, PAGING, BOOT_INFORMATION } type;
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
  uintptr_t physicalMemoryOffset;

  char cmdline[MAX_CMDLINE_LENGTH];

  MemoryMapEntry memoryMapEntries[MAX_MEMORY_MAP_ENTRIES_COUNT];
  size_t memoryMapEntriesCount;

  ModuleEntry moduleEntries[MAX_MODULE_ENTRIES_COUNT];
  size_t moduleEntriesCount;

  ReservedMemoryRegion reservedMemoryRegions[MAX_MEMORY_REGIONS_COUNT];
  size_t reservedMemoryRegionsCount;
};

