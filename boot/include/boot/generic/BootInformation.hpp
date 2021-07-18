#pragma once

#include <boot/generic/api/BootInformation.hpp>

namespace boot
{
  BootInformation initBootInformation(struct multiboot_boot_information* multiboot2BootInformation);
}
