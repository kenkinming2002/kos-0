#include <i686/boot/boot.hpp>

#include <i686/boot/Segmentation.hpp>
#include <i686/boot/memory/Paging.hpp>

__attribute__((aligned(4096))) std::byte kernelMemoryMapping[sizeof(boot::memory::MemoryMapping)];
__attribute__((aligned(4096))) std::byte kernelPageTable[sizeof(boot::memory::PageTable) * BOOT_PAGE_TABLE_COUNT];

__attribute__((aligned(4096))) std::byte kernelGDTEntries[sizeof(boot::GDTEntry) * GDT_SIZE];

