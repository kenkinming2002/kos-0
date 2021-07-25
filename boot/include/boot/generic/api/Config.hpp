#pragma once

#include <stddef.h>
#include <stdint.h>

static constexpr size_t BOOT_MEMORY_SIZE = 0x80000;

static constexpr size_t MAX_MEMORY_MAP_ENTRIES_COUNT = 0x10;
static constexpr size_t MAX_MODULE_ENTRIES_COUNT     = 0x10;
static constexpr size_t MAX_MEMORY_REGIONS_COUNT     = 0x10;
static constexpr size_t MAX_CMDLINE_LENGTH           = 0x20; // 32 Characters

constexpr uintptr_t PHYSICAL_MEMORY_MAPPING_START = 0xD0000000;
constexpr uintptr_t PHYSICAL_MEMORY_MAPPING_END   = 0xF0000000;
constexpr uintptr_t PHYSICAL_MEMORY_MAPPING_SIZE  = PHYSICAL_MEMORY_MAPPING_END - PHYSICAL_MEMORY_MAPPING_START;


