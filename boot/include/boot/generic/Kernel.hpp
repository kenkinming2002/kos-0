#pragma once

#include <boot/generic/multiboot2.hpp>
#include <boot/generic/api/multiboot2.h>

namespace boot
{
  void initializeKernel(struct multiboot_boot_information* multiboot2BootInformation);
}
