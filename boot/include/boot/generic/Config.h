#pragma once

#include <stddef.h>

static constexpr size_t BOOT_MEMORY_SIZE = 0x40000;

static constexpr size_t MAX_MEMORY_MAP_ENTRIES_COUNT = 0x10;
static constexpr size_t MAX_MODULE_ENTRIES_COUNT     = 0x10;
static constexpr size_t MAX_MEMORY_REGIONS_COUNT     = 0x10;
static constexpr size_t MAX_CMDLINE_LENGTH           = 0x20; // 32 Characters

