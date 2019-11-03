#include <generic/init/Multiboot2.hpp>

#include <stdint.h>

#define WEAK __attribute__((weak))

#define MULTIBOOT2_TAG_PARSE_STUB0(name) \
  WEAK int parse_##name##_tag() { return 0; }

#define MULTIBOOT2_TAG_PARSE_STUB1(name) \
  WEAK int parse_##name##_tag(struct multiboot_tag_##name * name) { return 0; }

#define MULTIBOOT2_TAG_PARSE_STUB2(name, struct_name) \
  WEAK int parse_##name##_tag(struct multiboot_tag_##struct_name * struct_name) { return 0; }

#define MULTIBOOT2_TAG_PARSE_CALL_ERROR_RETURN(exp) \
  if((status = exp)) return status

#define MULTIBOOT2_TAG_PARSE_CALL1(name) \
  MULTIBOOT2_TAG_PARSE_CALL_ERROR_RETURN(parse_##name##_tag())

#define MULTIBOOT2_TAG_PARSE_CALL2(name, tag) \
  MULTIBOOT2_TAG_PARSE_CALL_ERROR_RETURN(parse_##name##_tag(reinterpret_cast<struct multiboot_tag_##name*>(tag)))

#define MULTIBOOT2_TAG_PARSE_CALL3(name, struct_name, tag) \
  MULTIBOOT2_TAG_PARSE_CALL_ERROR_RETURN(parse_##name##_tag(reinterpret_cast<struct multiboot_tag_##struct_name*>(tag)))

namespace init::multiboot2
{
  MULTIBOOT2_TAG_PARSE_STUB2(cmdline, string)
  MULTIBOOT2_TAG_PARSE_STUB2(boot_loader_name, string)

  MULTIBOOT2_TAG_PARSE_STUB1(module)
  MULTIBOOT2_TAG_PARSE_STUB1(basic_meminfo)
  MULTIBOOT2_TAG_PARSE_STUB1(bootdev)
  MULTIBOOT2_TAG_PARSE_STUB1(mmap)
  MULTIBOOT2_TAG_PARSE_STUB1(vbe)
  MULTIBOOT2_TAG_PARSE_STUB1(framebuffer)
  MULTIBOOT2_TAG_PARSE_STUB1(elf_sections)
  MULTIBOOT2_TAG_PARSE_STUB1(apm)
  MULTIBOOT2_TAG_PARSE_STUB1(efi32)
  MULTIBOOT2_TAG_PARSE_STUB1(efi64)
  MULTIBOOT2_TAG_PARSE_STUB1(smbios)
  MULTIBOOT2_TAG_PARSE_STUB1(old_acpi)
  MULTIBOOT2_TAG_PARSE_STUB1(new_acpi)
  MULTIBOOT2_TAG_PARSE_STUB1(network)
  MULTIBOOT2_TAG_PARSE_STUB1(efi_mmap)

  MULTIBOOT2_TAG_PARSE_STUB0(efi_bs)

  MULTIBOOT2_TAG_PARSE_STUB1(efi32_ih)
  MULTIBOOT2_TAG_PARSE_STUB1(efi64_ih)
  MULTIBOOT2_TAG_PARSE_STUB1(load_base_addr)

  int parseBootInformation(void *boot_information)
  {
    int status;
    for(struct multiboot_tag *tag = (struct multiboot_tag *)(boot_information + 8);
        tag->type != MULTIBOOT_TAG_TYPE_END;
        tag = (struct multiboot_tag *) ((uint8_t*)tag + ((tag->size+7) & ~7))
       )
    {
      switch(tag->type)
      {
      case MULTIBOOT_TAG_TYPE_CMDLINE:          MULTIBOOT2_TAG_PARSE_CALL3(cmdline,          string, tag); break;
      case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: MULTIBOOT2_TAG_PARSE_CALL3(boot_loader_name, string, tag); break;

      case MULTIBOOT_TAG_TYPE_MODULE:           MULTIBOOT2_TAG_PARSE_CALL2(module,                   tag); break;
      case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:    MULTIBOOT2_TAG_PARSE_CALL2(basic_meminfo,            tag); break;
      case MULTIBOOT_TAG_TYPE_BOOTDEV:          MULTIBOOT2_TAG_PARSE_CALL2(bootdev,                  tag); break;
      case MULTIBOOT_TAG_TYPE_MMAP:             MULTIBOOT2_TAG_PARSE_CALL2(mmap,                     tag); break;
      case MULTIBOOT_TAG_TYPE_VBE:              MULTIBOOT2_TAG_PARSE_CALL2(vbe,                      tag); break;
      case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:      MULTIBOOT2_TAG_PARSE_CALL2(framebuffer,              tag); break;
      case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:     MULTIBOOT2_TAG_PARSE_CALL2(elf_sections,             tag); break;
      case MULTIBOOT_TAG_TYPE_APM:              MULTIBOOT2_TAG_PARSE_CALL2(apm,                      tag); break;
      case MULTIBOOT_TAG_TYPE_EFI32:            MULTIBOOT2_TAG_PARSE_CALL2(efi32,                    tag); break;
      case MULTIBOOT_TAG_TYPE_EFI64:            MULTIBOOT2_TAG_PARSE_CALL2(efi64,                    tag); break;
      case MULTIBOOT_TAG_TYPE_SMBIOS:           MULTIBOOT2_TAG_PARSE_CALL2(smbios,                   tag); break;
      case MULTIBOOT_TAG_TYPE_ACPI_OLD:         MULTIBOOT2_TAG_PARSE_CALL2(old_acpi,                 tag); break;
      case MULTIBOOT_TAG_TYPE_ACPI_NEW:         MULTIBOOT2_TAG_PARSE_CALL2(new_acpi,                 tag); break;
      case MULTIBOOT_TAG_TYPE_NETWORK:          MULTIBOOT2_TAG_PARSE_CALL2(network,                  tag); break;
      case MULTIBOOT_TAG_TYPE_EFI_MMAP:         MULTIBOOT2_TAG_PARSE_CALL2(efi_mmap,                 tag); break;

      case MULTIBOOT_TAG_TYPE_EFI_BS:           MULTIBOOT2_TAG_PARSE_CALL1(efi_bs                       ); break;

      case MULTIBOOT_TAG_TYPE_EFI32_IH:         MULTIBOOT2_TAG_PARSE_CALL2(efi32_ih,                 tag); break;
      case MULTIBOOT_TAG_TYPE_EFI64_IH:         MULTIBOOT2_TAG_PARSE_CALL2(efi64_ih,                 tag); break;
      case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR:   MULTIBOOT2_TAG_PARSE_CALL2(load_base_addr,           tag); break;
      }
    }

    return 0;
  }
}
