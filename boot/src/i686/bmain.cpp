#include <boot/generic/multiboot2.h>
#include <boot/generic/BootInformation.hpp>
#include <boot/generic/Kernel.hpp>
#include <boot/generic/Panic.hpp>
#include <boot/i686/Paging.hpp>

extern "C" void bmain(struct multiboot_boot_information* multiboot2BootInformation)
{
  boot::initKernel(multiboot2BootInformation);
  boot::memory::initPaging();
  auto* bootInformation = boot::initBootInformation(multiboot2BootInformation);
  if(!bootInformation)
    panic("Failed to create boot information");

  boot::runKernel(*bootInformation);
}

