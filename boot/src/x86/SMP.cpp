#include <boot/x86/SMP.hpp>

#include <common/x86/ACPI.hpp>

#include <librt/Assert.hpp>
#include <librt/Panic.hpp>
#include <librt/Log.hpp>
#include <librt/StringRef.hpp>

#include <stddef.h>
#include <stdint.h>

namespace boot
{
  static constexpr uintptr_t EBDA_BEGIN = 0x00080000;
  static constexpr uintptr_t EBDA_END   = 0x000A0000;

  static constexpr uintptr_t MAIN_BIOS_AREA_BEGIN = 0x000E0000;
  static constexpr uintptr_t MAIN_BIOS_AREA_END   = 0x00100000;

  template<typename T>
  static uint8_t checksum(const T* value)
  {
    uint8_t result = 0;
    for(size_t i=0; i<sizeof *value; ++i)
      result += reinterpret_cast<const char*>(value)[i];
    return result;
  }

  static RSDPBase* findRSDP()
  {
    for(uintptr_t addr = EBDA_BEGIN; addr != EBDA_END; addr += 16) // Always lie on 16-byte boundary
      if(auto* rsdp = reinterpret_cast<RSDPBase*>(addr); RSDPValidate(rsdp))
        return rsdp;

    for(uintptr_t addr = MAIN_BIOS_AREA_BEGIN; addr != MAIN_BIOS_AREA_END; addr += 16) // Always lie on 16-byte boundary
      if(auto* rsdp = reinterpret_cast<RSDPBase*>(addr); RSDPValidate(rsdp))
        return rsdp;

    return nullptr;
  }

  static MADT* findMADT(RSDPBase* rsdp)
  {
    switch(rsdp->revision)
    {
      case 0:
      {
        auto* rsdp10 = static_cast<RSDP10*>(rsdp);
        auto* rsdt = reinterpret_cast<RSDT*>(rsdp10->rsdtAddress);
        auto count = RSDTCount(rsdt);
        for(size_t i=0; i<count; ++i)
        {
          auto* sdt = reinterpret_cast<SDT*>(rsdt->sdts[i]);
          auto signature = rt::StringRef(sdt->signature, sizeof sdt->signature);
          if(signature == "APIC")
            return static_cast<MADT*>(sdt);
        }
        break;
      }
      default:
        ASSERT_UNIMPLEMENTED;
    }
    return nullptr;
  }

  struct Core
  {
    uint8_t localAPICID;

    bool onlineCapable : 1;
    bool isBSP : 1; // Set if is bootstrap processor
  };

  struct SMPInfo
  {
    Core cores[256];
    uint8_t coresCount;

    // OSDEV tutorial use uint64_t, however, we cannot really access 64 bit
    // pointer on a 32 bit system anyway, so using 64 bit value is pointness.
    uintptr_t localAPICPtr;
  };

  static SMPInfo parseMADT(const MADT* madt)
  {
    SMPInfo smpInfo{};

    smpInfo.localAPICPtr = madt->localAPICAddress;

    /* Note: The osdev tutorial titled Symmetric Multiptrocessing has a bug in
     *       that it is missing clober for eax, ecx and edx, which leads to
     *       undefined behavior that is especially troublesome when combined
     *       with GCC register calling convention with static linkage. */
    uint8_t bspid;
    asm volatile ("mov eax, 0x1; cpuid; shr ebx, 0x18" : "=b"(bspid) : : "eax", "ecx", "edx");

    for(auto* madtEntry = madt->entries; madtEntry<MADTEntryEnd(madt); madtEntry = MADTEntryNext(madtEntry))
      switch(madtEntry->type)
      {
        case MADTEntry::Type::PROCESSOR_LOCAL_APIC:
        {
          auto* entry = static_cast<const ProcessorLocalAPICMADTEntry*>(madtEntry);
          auto& core = smpInfo.cores[smpInfo.coresCount++];
          core.localAPICID   = entry->APICID;
          core.onlineCapable = entry->flag & 1;
          core.isBSP         = core.localAPICID == bspid;
          break;
        }
        case MADTEntry::Type::LOCAL_APIC_ADDRESS_OVERRIDE:
          rt::panic("64 bit override for local APIC address found but system is 32 bit\n");
        default:
          break;
      }

    return smpInfo;
  }

  static void writeLocalAPIC(const SMPInfo& smpInfo, size_t offset, uint32_t value) { *reinterpret_cast<volatile uint32_t*>(smpInfo.localAPICPtr + offset) = value; }
  static uint32_t readLocalAPIC(const SMPInfo& smpInfo, size_t offset)              { return *reinterpret_cast<volatile uint32_t*>(smpInfo.localAPICPtr + offset); }

  [[gnu::noinline]] void busyWait(size_t count)
  {
      // Busy wait
      for(size_t i=0; i<count; ++i)
        asm volatile ("": : : "memory");
  }

  extern "C" { volatile uint8_t APRunning; }
  static void startupAPs(SMPInfo smpInfo)
  {
    rt::logf("Found number of core:%d\n", int(smpInfo.coresCount));
    rt::logf("Local APIC address:0x%lx\n", smpInfo.localAPICPtr);

    for(size_t i=0; i<smpInfo.coresCount; ++i)
    {
      const auto& core = smpInfo.cores[i];
      if(core.isBSP)
        continue; // Do not attempt to restart BSP since we are running on it

      if(!core.onlineCapable)
        continue;

      // SEND Init IPI
      writeLocalAPIC(smpInfo, 0x280, 0); // Clear APIC error

      writeLocalAPIC(smpInfo, 0x310, (readLocalAPIC(smpInfo, 0x310) & 0x00FFFFFF) | (core.localAPICID << 24)); // Select AP
      writeLocalAPIC(smpInfo, 0x300, (readLocalAPIC(smpInfo, 0x300) & 0xFFF00000) | 0x0C500);                  // Trigger Init IPI
      do { asm volatile("pause" : : : "memory"); } while(readLocalAPIC(smpInfo, 0x300) & (1 << 12));           // Wait for delivery

      writeLocalAPIC(smpInfo, 0x310, (readLocalAPIC(smpInfo, 0x310) & 0x00FFFFFF) | (core.localAPICID << 24)); // Select AP
      writeLocalAPIC(smpInfo, 0x300, (readLocalAPIC(smpInfo, 0x300) & 0xFFF00000) | 0x08500);                  // deassert
      do { asm volatile("pause" : : : "memory"); } while(readLocalAPIC(smpInfo, 0x300) & (1 << 12));           // Wait for delivery

      // Busy wait
      busyWait(2000);

      // SEND Startup IPI(twice)
      for(size_t j=0; j<2; ++j)
      {
        writeLocalAPIC(smpInfo, 0x280, 0); // Clear APIC error
        writeLocalAPIC(smpInfo, 0x310, (readLocalAPIC(smpInfo, 0x310) & 0x00FFFFFF) | (core.localAPICID << 24)); // Select AP
        writeLocalAPIC(smpInfo, 0x300, (readLocalAPIC(smpInfo, 0x300) & 0xFFF0F800) | 0x000608);   // Trigger Startup IPI for 0x800:0000

        busyWait(2000);

        do { asm volatile("pause" : : : "memory"); } while(readLocalAPIC(smpInfo, 0x300) & (1 << 12));
      }
    }

    rt::logf("Waiting for %d AP to start...\n", smpInfo.coresCount - 1);
    do { asm volatile("pause" : : : "memory"); } while(APRunning != smpInfo.coresCount - 1);
    rt::logf("All %d AP successfully started\n", APRunning);
  }

  namespace { uint8_t coresCount; }
  void initializeSMP()
  {
    rt::logf("Finding rsdp descriptor...\n");
    auto* rsdp = findRSDP();
    if(!rsdp)
      rt::panic("Failed to find rsdp descriptor\n");

    auto* madt = findMADT(rsdp);
    if(!madt)
      rt::panic("Failed to find MADT\n");

    auto smpInfo = parseMADT(madt);
    startupAPs(smpInfo);

    coresCount = smpInfo.coresCount;
  }

  extern "C" { volatile uint8_t APCanContinue; }
  void apContinue() { APCanContinue = 1; }

  uint8_t getCoresCount()
  {
    return coresCount;
  }
}
