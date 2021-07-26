#pragma once

#include <stddef.h>
#include <stdint.h>

static constexpr size_t BOOT_MEMORY_SIZE = 0x400000;

constexpr uintptr_t PHYSICAL_MEMORY_MAPPING_START = 0xD0000000;
constexpr uintptr_t PHYSICAL_MEMORY_MAPPING_END   = 0xF0000000;
constexpr uintptr_t PHYSICAL_MEMORY_MAPPING_SIZE  = PHYSICAL_MEMORY_MAPPING_END - PHYSICAL_MEMORY_MAPPING_START;


