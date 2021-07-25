#include <boot/generic/Memory.hpp>

#include <boot/generic/api/Config.hpp>

#include <common/i686/memory/Paging.hpp>

#include <librt/Panic.hpp>

namespace boot::memory
{
  using common::memory::PAGE_SIZE;

  namespace
  {
    struct alignas(PAGE_SIZE) Page { char data[PAGE_SIZE]; };

    static constexpr size_t BOOT_MEMORY_PAGES_COUNT = (BOOT_MEMORY_SIZE + (PAGE_SIZE-1)) / PAGE_SIZE;
    static Page bootMemory[BOOT_MEMORY_PAGES_COUNT];

    static size_t index = 0;
  }

  void* allocPages(size_t count)
  {
    if(index+count>=BOOT_MEMORY_PAGES_COUNT)
      rt::panic("Out of boot memory\n");

    auto result = static_cast<void*>(&bootMemory[index]);
    index += count;
    return result;
  }

  void* alloc(size_t size)
  {
    auto count = (size + (PAGE_SIZE-1)) / PAGE_SIZE;
    return allocPages(count);
  }

  Region getUsedRegion() { return Region{.addr = reinterpret_cast<uintptr_t>(bootMemory), .length = index * PAGE_SIZE}; }
}

