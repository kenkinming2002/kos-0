#pragma once

#include "common/i686/memory/Paging.hpp"
#include <stdint.h>
#include <stddef.h>

#define BOOT_FUNCTION [[gnu::section(".boot.text")]]

//  TODO: set this depending on kernel size
constexpr size_t BOOT_PAGE_TABLE_COUNT = 1;

// extern char initialPageDirectory[];
// extern char kernelPageTables[];

extern common::memory::PageDirectory initialPageDirectory;
extern common::memory::PageTable     kernelPageTables[BOOT_PAGE_TABLE_COUNT];
