#include <x86/acpi/ACPI.hpp>

#include <generic/memory/Memory.hpp>

#include <librt/StringRef.hpp>
#include <librt/Log.hpp>
#include <librt/Panic.hpp>
#include <librt/Strings.hpp>

namespace core::acpi
{
  static constexpr memory::physaddr_t EBDA_BEGIN = 0x00080000;
  static constexpr memory::physaddr_t EBDA_END   = 0x000A0000;

  static constexpr memory::physaddr_t MAIN_BIOS_AREA_BEGIN = 0x000E0000;
  static constexpr memory::physaddr_t MAIN_BIOS_AREA_END   = 0x00100000;

  static RSDPBase* findRSDP()
  {
    for(memory::physaddr_t physaddr = EBDA_BEGIN; physaddr != EBDA_END; physaddr += 16) // Always lie on 16-byte boundary
      if(auto* rsdp = reinterpret_cast<RSDPBase*>(memory::physToVirt(physaddr)); RSDPValidate(rsdp))
        return rsdp;

    for(memory::physaddr_t physaddr = MAIN_BIOS_AREA_BEGIN; physaddr != MAIN_BIOS_AREA_END; physaddr += 16) // Always lie on 16-byte boundary
      if(auto* rsdp = reinterpret_cast<RSDPBase*>(memory::physToVirt(physaddr)); RSDPValidate(rsdp))
        return rsdp;

    return nullptr;
  }

  namespace
  {
#if __LP64__
    XSDT* xsdt = nullptr;
#endif

    RSDT* rsdt = nullptr;
  }

  static void parseRSDP(RSDPBase* rsdp)
  {
#if __LP64__
    if(rsdp->revision>=2)
    {
      auto* rsdp20 = static_cast<RSDP20*>(rsdp);
      xsdt = reinterpret_cast<XSDT*>(memory::physToVirt(rsdp20->xsdtAddress));
      if(SDTValidate(xsdt))
        return;
    }
#endif

    if(rsdp->revision>=0)
    {
      auto* rsdp10 = static_cast<RSDP10*>(rsdp);
      rsdt = reinterpret_cast<RSDT*>(memory::physToVirt(rsdp10->rsdtAddress));
      if(SDTValidate(rsdt))
        return;
    }

    rt::panic("Invalid rsdp\n");
  }

  void initialize()
  {
    rt::log("Finding rsdp descriptor...\n");
    auto* rsdp = findRSDP();
    if(!rsdp)
      rt::panic("Failed to find rsdp descriptor\n");

    parseRSDP(rsdp);
    rt::log("Done\n");
  }

  const SDT* findSDT(rt::StringRef name)
  {
#if __LP64__
    if(xsdt)
    {
      for(size_t i=0; i<XSDTCount(xsdt); ++i)
      {
        auto* sdt = reinterpret_cast<const SDT*>(memory::physToVirt(xsdt->sdts[i]));
        if(rt::StringRef(sdt->signature, sizeof sdt->signature) == name)
          return sdt;
      }
      return nullptr;
    }
#endif

    if(rsdt)
    {
      for(size_t i=0; i<RSDTCount(rsdt); ++i)
      {
        auto* sdt = reinterpret_cast<const SDT*>(memory::physToVirt(rsdt->sdts[i]));
        if(rt::StringRef(sdt->signature, sizeof sdt->signature) == name)
          return sdt;
      }
      return nullptr;
    }

    ASSERT_UNREACHABLE;
  }
}
