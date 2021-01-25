#pragma once

#include <stddef.h>

static constexpr size_t BOOT_INFORMATION_STORAGE_SIZE = 0x1000;  // 4KiB
static constexpr size_t KERNEL_IMAGE_STORAGE_SIZE     = 0x20000; // 64KiB

static constexpr size_t PAGE_TABLE_COUNT = 0x10;

static constexpr size_t MAX_MEMORY_MAP_ENTRIES_COUNT = 0x10;
static constexpr size_t MAX_MODULE_ENTRIES_COUNT     = 0x10;
static constexpr size_t MAX_MEMORY_REGIONS_COUNT     = 0x10;
static constexpr size_t MAX_CMDLINE_LENGTH           = 0x20; // 32 Characters

