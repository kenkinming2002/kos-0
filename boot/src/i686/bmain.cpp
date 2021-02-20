#include <boot/generic/multiboot2.h>
#include <boot/generic/BootInformation.hpp>
#include <boot/generic/Kernel.hpp>
#include <boot/generic/Memory.hpp>
#include <boot/i686/Paging.hpp>

#include <librt/Panic.hpp>

extern "C" void bmain(struct multiboot_boot_information* multiboot2BootInformation)
{
  // BootInformation
  BootInformation bootInformation = {};

  // Paging
  boot::memory::initializePaging(bootInformation);

  // Kernel
  auto kernel = boot::Kernel::from(multiboot2BootInformation);
  if(!kernel)
    rt::panic("Failed to locate kernel\n");

  if(kernel->extractAndMap(bootInformation) != 0)
    rt::panic("Failed to extract and map kernel\n");

  boot::bootInformationInitialize(bootInformation, multiboot2BootInformation);

  kernel->run(bootInformation);
}

