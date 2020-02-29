#include <generic/core/Process.hpp>

#include <i686/boot/boot.hpp>
#include <i686/core/Segmentation.hpp>
#include <i686/core/Syscall.hpp>
#include <generic/core/Memory.hpp>

#include <generic/utils/Utilities.hpp>

#include <i686/asm/Process.hpp>

#include <string.h>

#include <i686/core/MultiProcessing.hpp>

#include <generic/io/Print.hpp>

namespace core
{
  namespace
  {
    void startProcess()
    {
      auto& currentProcess = multiprocessing::processesList.front();
      currentProcess.setStackAsActive();
      currentProcess.enterUserMode();
    }
  }

  Process::Process(uintptr_t kernelStack, uintptr_t startAddress) 
    : context{kernelStack, {}},
      startAddress(startAddress),
      kernelStack(kernelStack), 
      kernelStackSegmentSelector(0x10)
  {
    io::print("DEBUG: Process Creation\n");
    io::print("  - kernel stack address ", kernelStack, "\n");
    io::print("  - user process start address ", startAddress, "\n");

    // Put the return address on the stack
    kernelStack+=sizeof(uintptr_t);
    *reinterpret_cast<uintptr_t*>(kernelStack) = reinterpret_cast<uintptr_t>(&startProcess);
  }

  void Process::setStackAsActive() const
  {
    segmentation::taskStateSegment.esp0 = kernelStack;
    segmentation::taskStateSegment.ss0 = kernelStackSegmentSelector;

    core::set_syscall_esp(kernelStack);
  }

  void Process::addSection(memory::virtaddr_t virtualAddress, memory::Access access, memory::Permission permission,
      const uint8_t* content, size_t length)
  {
    size_t pageCount = (length + memory::PAGE_SIZE - 1) / memory::PAGE_SIZE;

    auto physicalMemoryRegion = core::memory::allocatePhysicalMemoryRegion(pageCount);
    if(!physicalMemoryRegion)
      return;

    auto virtualMemoryRegion = memory::MemoryRegion(virtualAddress, pageCount * memory::PAGE_SIZE, memory::MemoryRegion::address_length_tag);
    context.memoryMapping.map(*physicalMemoryRegion, virtualMemoryRegion, access, permission);

    // Memory is mapped here 
    memcpy(reinterpret_cast<void*>(virtualAddress), reinterpret_cast<const void*>(content), length);
  }

  void Process::enterUserMode() const
  {
    asm volatile (R"(
      .intel_syntax noprefix
        push 0x23
        push 0x0

        pushf
        pop eax
        or eax, 0x200
        push eax

        push 0x1B
        push %[startAddress]

        iret
      .att_syntax prefix
    )" : : [startAddress]"r"(startAddress) : "eax");
    __builtin_unreachable();
  }
}
