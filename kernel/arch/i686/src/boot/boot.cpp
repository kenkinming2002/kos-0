#include <i686/boot/boot.hpp>

#include <i686/core/Segmentation.hpp>
#include <i686/boot/memory/Paging.hpp>

__attribute__((aligned(4096))) std::byte kernelPageDirectory[sizeof(boot::memory::PageDirectory)];
__attribute__((aligned(4096))) std::byte kernelPageTable[sizeof(boot::memory::PageTable) * BOOT_PAGE_TABLE_COUNT];

core::GDTEntry kernelGDTEntries[GDT_SIZE];
core::TaskStateSegment kernelTaskStateSegment;

