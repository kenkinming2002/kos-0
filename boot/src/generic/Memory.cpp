#include <boot/generic/Memory.hpp>

#include <boot/generic/api/Config.hpp>

#include <common/i686/memory/Paging.hpp>

#include <librt/Panic.hpp>

namespace boot::memory
{
  using namespace common::memory;

  void* allocPages(BootInformation& bootInformation, size_t count, ReservedMemoryRegion::Type type)
  {
    struct alignas(PAGE_SIZE) Page { char data[PAGE_SIZE]; };

    static constexpr size_t BOOT_MEMORY_PAGES_COUNT = (BOOT_MEMORY_SIZE + (PAGE_SIZE-1)) / PAGE_SIZE;
    static Page bootMemory[BOOT_MEMORY_PAGES_COUNT];
    static size_t index = 0;

    if(index+count>=BOOT_MEMORY_PAGES_COUNT)
      rt::panic("Out of boot memory\n");

    auto result = static_cast<void*>(&bootMemory[index]);
    index += count;

    auto reservedMemoryRegion = ReservedMemoryRegion{.addr = reinterpret_cast<uintptr_t>(result), .len = count * PAGE_SIZE, .type = type};
    bootInformation.reservedMemoryRegions[bootInformation.reservedMemoryRegionsCount++] = reservedMemoryRegion;
    return result;
  }

  void* alloc(BootInformation& bootInformation, size_t size, ReservedMemoryRegion::Type type)
  {
    auto count = (size + (PAGE_SIZE-1)) / PAGE_SIZE;
    return allocPages(bootInformation, count, type);
  }

}

