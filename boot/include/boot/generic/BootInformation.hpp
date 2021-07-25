#pragma once

#include <boot/generic/api/multiboot2.h>
#include <stdint.h>

namespace boot
{
  void initializeBootInformation(struct multiboot_boot_information* multiboot2BootInformation);
}
