#include <boot/i686/Boot.hpp>

#include <common/i686/memory/Paging.hpp>

alignas(4096) char initialPageDirectory[sizeof(common::memory::PageDirectory)];
alignas(4096) char kernelPageTables[sizeof(common::memory::PageTable) * BOOT_PAGE_TABLE_COUNT];

