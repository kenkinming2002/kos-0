#pragma once

#include <librt/StringRef.hpp>

#include <stddef.h>
#include <stdint.h>

struct [[gnu::packed]] RSDPBase
{
  char signature[8];
  uint8_t checksum;
  char oemid[6];
  uint8_t revision;
};

struct [[gnu::packed]] RSDP10 : public RSDPBase
{
  uint32_t rsdtAddress;
};

struct [[gnu::packed]] RSDP20 : public RSDP10
{
  uint32_t length;
  uint64_t xsdtAddress;
  uint8_t extendedChecksum;
  uint8_t reserved[3];
};

template<typename T>
inline uint8_t RSDPChecksum(const T* value) requires std::is_base_of_v<RSDPBase, T>
{
  uint8_t result = 0;
  for(size_t i=0; i<sizeof *value; ++i)
    result += reinterpret_cast<const char*>(value)[i];
  return result;
}

inline bool RSDPValidate(const RSDPBase* rsdp)
{
  auto signature = rt::StringRef(rsdp->signature, sizeof rsdp->signature);
  return signature == "RSD PTR "
    && (rsdp->revision < 0 || RSDPChecksum(static_cast<const RSDP10*>(rsdp)) == 0)
    && (rsdp->revision < 1 || RSDPChecksum(static_cast<const RSDP20*>(rsdp)) == 0);
}

struct [[gnu::packed]] SDT
{
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oemid[6];
  char oemTableID[8];
  uint32_t oemRevision;
  uint32_t creatorID;
  uint32_t creatorRevision;
};

inline uint8_t SDTChecksum(const SDT* sdt)
{
  uint8_t result = 0;
  for(size_t i=0; i<sdt->length; ++i)
    result += reinterpret_cast<const char*>(sdt)[i];
  return result;
}

inline bool SDTValidate(const SDT* sdt)
{
  return SDTChecksum(sdt) == 0;
}

struct [[gnu::packed]] RSDT : public SDT
{
  uint32_t sdts[];
};

constexpr size_t RSDTCount(RSDT* rsdt)
{
  return (rsdt->length - sizeof *rsdt) / sizeof rsdt->sdts[0];
}

struct [[gnu::packed]] XSDT : public SDT
{
  uint64_t sdts[];
};

constexpr size_t XSDTCount(XSDT* xsdt)
{
  return (xsdt->length - sizeof *xsdt) / sizeof xsdt->sdts[0];
}


struct [[gnu::packed]] MADTEntry
{
  enum class Type : uint8_t
  {
    PROCESSOR_LOCAL_APIC = 0,

    IO_APIC                           = 1,
    IO_APIC_INTERRUPT_SOURCE_OVERRIDE = 2,
    IO_APIC_NMI_SOURCE                = 3,

    LOCAL_APIC_NMI              = 4,
    LOCAL_APIC_ADDRESS_OVERRIDE = 5,
    PROCESSOR_LOCAL_X2APIC      = 9
  };

  Type type;
  uint8_t length;
};

struct [[gnu::packed]] ProcessorLocalAPICMADTEntry : public MADTEntry
{
  uint8_t ACPIProcessorID;
  uint8_t APICID;
  uint32_t flag; // Bit 1 = Online Capable
};

struct [[gnu::packed]] IOAPICMADTEntry : public MADTEntry
{
  uint8_t IOAPICID;
  uint8_t reserved;
  uint32_t IOAPICAddress;
  uint32_t gsiBase;
};

struct [[gnu::packed]] IOAPICInterruptSourceOverrideMADTEntry : public MADTEntry
{
  uint8_t busSource;
  uint8_t irqSource;
  uint32_t gsi;
  uint16_t flags;
};

struct [[gnu::packed]] IOAPICNMISourceMADTEntry : public MADTEntry
{
  uint8_t nmiSource;
  uint8_t reserved;
  uint32_t gsi;
  uint16_t flags;
};

struct [[gnu::packed]] LOCALAPICNMIMADTEntry : public MADTEntry
{
  uint8_t ACPIProcessorID;
  uint16_t flags;
  uint8_t lint;
};

struct [[gnu::packed]] LocalAPICAddressOverrideMADTEntry : public MADTEntry
{
  uint16_t reserved;
  uint64_t localAPICAddressOverride;
};

struct [[gnu::packed]] ProcessorLocalX2APICMADTEntry : public MADTEntry
{
  uint16_t reserved;
  uint32_t processorLocalX2APICID;
  uint32_t flags;
  uint32_t ACPIID;
};

struct [[gnu::packed]] MADT : public SDT
{
  uint32_t localAPICAddress;
  uint32_t flag; // 1 = Dual8259 legacy PICs installed
  MADTEntry entries[];
};

inline const MADTEntry* MADTEntryNext(const MADTEntry* madtEntry) { return reinterpret_cast<MADTEntry*>(reinterpret_cast<uintptr_t>(madtEntry)+madtEntry->length); }
inline const MADTEntry* MADTEntryEnd(const MADT* madt)            { return reinterpret_cast<MADTEntry*>(reinterpret_cast<uintptr_t>(madt)+madt->length); }

