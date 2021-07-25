#include <boot/generic/api/multiboot2.h>

#include <boot/generic/BootInformation.hpp>
#include <boot/generic/Kernel.hpp>
#include <boot/generic/Memory.hpp>
#include <boot/x86/SMP.hpp>
#include <boot/i686/Paging.hpp>
#include <boot/i686/Entry.hpp>

#include <librt/Panic.hpp>

extern "C" void bmain(struct multiboot_boot_information* multiboot2BootInformation)
{
  boot::initializeKernel(multiboot2BootInformation);
  boot::memory::initializePaging();
  boot::initializeSMP();

  boot::initializeBootInformation(multiboot2BootInformation); // Must be done at last
  boot::apContinue();
  entry();
}

