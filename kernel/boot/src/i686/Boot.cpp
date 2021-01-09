#include <boot/i686/Boot.hpp>

#include <common/i686/memory/Paging.hpp>

alignas(4096) constinit common::memory::PageDirectory initialPageDirectory;
alignas(4096) constinit common::memory::PageTable     kernelPageTables[BOOT_PAGE_TABLE_COUNT];

