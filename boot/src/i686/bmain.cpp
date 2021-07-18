#include <boot/generic/api/multiboot2.h>

#include <boot/generic/BootInformation.hpp>
#include <boot/generic/Kernel.hpp>
#include <boot/generic/Memory.hpp>
#include <boot/i686/Paging.hpp>

#include <librt/Panic.hpp>

extern "C" void bmain(struct multiboot_boot_information* multiboot2BootInformation)
{
  auto bootInformation = boot::initBootInformation(multiboot2BootInformation);

  boot::memory::initializePaging(bootInformation);
  boot::Kernel kernel(bootInformation);

  if(!kernel.extractAndMap(bootInformation))
    rt::panic("Failed to extract and map kernel\n");

  kernel.run(bootInformation);
}

