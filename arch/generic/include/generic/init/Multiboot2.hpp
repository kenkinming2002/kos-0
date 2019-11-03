#pragma once

#include <generic/grub/multiboot2.h>

namespace init::multiboot2
{
  int parseBootInformation(void* parseBootInformation);
}
