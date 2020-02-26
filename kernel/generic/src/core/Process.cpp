#include <generic/core/Process.hpp>

#include <i686/boot/boot.hpp>
#include <i686/core/Segmentation.hpp>
#include <i686/core/Syscall.hpp>
#include <generic/core/Memory.hpp>

#include <generic/utils/Utilities.hpp>

#include <string.h>

extern "C"
{
  extern std::byte kernel_stack[];
}

namespace core
{
  Process::Process() 
    : m_kernelStack(reinterpret_cast<uintptr_t>(kernel_stack)), m_kernelStackSegmentSelector(0x10) {}

  void Process::setAsActive() const
  {
    m_memoryMapping.setAsActive();

    segmentation::taskStateSegment.esp0 = m_kernelStack;
    segmentation::taskStateSegment.ss0 = m_kernelStackSegmentSelector;

    core::set_syscall_esp(m_kernelStack);
  }

  void Process::addSection(memory::virtaddr_t virtualAddress, memory::Access access, memory::Permission permission,
      const uint8_t* content, size_t length)
  {
    size_t pageCount = (length + memory::PAGE_SIZE - 1) / memory::PAGE_SIZE;

    auto physicalMemoryRegion = core::memory::allocatePhysicalMemoryRegion(pageCount);
    if(!physicalMemoryRegion)
      return;

    auto virtualMemoryRegion = memory::MemoryRegion(virtualAddress, pageCount * memory::PAGE_SIZE, memory::MemoryRegion::address_length_tag);
    m_memoryMapping.map(*physicalMemoryRegion, virtualMemoryRegion, access, permission);

    // Memory is mapped here 
    memcpy(reinterpret_cast<void*>(virtualAddress), reinterpret_cast<const void*>(content), length);
  }

  void Process::run() const
  {
    // Pretend to return from a interrupt
    // NOTE: This can also be implemented with sysexit instead of iret
    asm volatile ( R"(
      .intel_syntax noprefix
        cli

        mov ax, 0x23
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax

        push 0x23
        push 0x00000000

        pushf
        pop eax
        or eax, 0x200
        push eax

        push 0x1B
        push 0x00000000
        iret
      .att_syntax prefix
      )"
      :
      : 
      : "eax"
    );
    __builtin_unreachable();
  }
}
