#pragma once

#include <common/x86/ACPI.hpp>

#include <librt/StringRef.hpp>

#include <stdint.h>
#include <stddef.h>

namespace core::acpi
{
  void initialize();
  const SDT* findSDT(rt::StringRef name);
}
