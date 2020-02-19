#include <i686/boot/boot.hpp>

#include <i686/boot/Segmentation.hpp>
#include <i686/boot/Paging.hpp>

BootInformation bootInformation;

__attribute__((aligned(4096))) std::byte kernelPageDirectory[sizeof(boot::PageDirectory)];
__attribute__((aligned(4096))) std::byte kernelPageTable[sizeof(boot::PageTable) * BOOT_PAGE_TABLE_COUNT];

__attribute__((aligned(4096))) std::byte kernelGDTEntries[sizeof(boot::GDTEntry) * GDT_SIZE];

