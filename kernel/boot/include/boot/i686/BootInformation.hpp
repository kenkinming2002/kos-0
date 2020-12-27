#pragma once

#include <stdint.h>
#include <stddef.h>

#include "Boot.hpp"

#include "grub/multiboot2.h"

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
};

struct BootInformation
{
public:
  MemoryMapEntry* memoryMapEntries;
  size_t memoryMapEntriesCount;

public:
  ModuleEntry* moduleEntries;
  size_t moduleEntriesCount;

public:
  struct multiboot_tag_framebuffer frameBuffer;
};

/*
 * BootInformation parsed in lower half is stored at the top of stack. This
 * spare us from allocating a region for storing it abd freeing it subsequently
 * since it will *eventually* be overwriiten if needed, but since the stack is
 * 4KiB large(as of now), the BootInformation struct will *likely* survive long
 * enough.
 */
extern char bootInformationStorageBegin[];
#define BOOT_INFORMATION (*reinterpret_cast<BootInformation*>(bootInformationStorageBegin))
