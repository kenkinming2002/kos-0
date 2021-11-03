#include <x86/LocalAPIC.hpp>

#include <x86/acpi/ACPI.hpp>

#include <librt/Panic.hpp>

namespace core
{
  namespace
  {
    core::memory::physaddr_t physaddr;
  }

  void LocalAPIC::initialize()
  {
    auto* madt = static_cast<const MADT*>(core::acpi::findSDT("APIC"));
    if(!madt)
      rt::panic("Failed to find MADT\n");

    physaddr = madt->localAPICAddress;
  }

  void LocalAPIC::write(size_t offset, uint32_t data)
  {
    ASSERT(physaddr != 0);
    core::memory::KMapGuard guard(physaddr);
    *reinterpret_cast<uint32_t*>(guard.addr+offset) = data;
  }

  uint32_t LocalAPIC::read(size_t offset)
  {
    ASSERT(physaddr != 0);
    core::memory::KMapGuard guard(physaddr);
    return *reinterpret_cast<uint32_t*>(guard.addr+offset);
  }
}

