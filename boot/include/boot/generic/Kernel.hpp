#pragma once

#include <boot/generic/BootInformation.hpp>
#include <boot/generic/multiboot2.h>
#include <libelf/libelf.hpp>

#include <librt/Optional.hpp>

#include <stddef.h>

namespace boot
{
  class Kernel
  {
  public:
    static rt::Optional<Kernel> from(const multiboot_boot_information* multiboot2BootInformation);

  public:
    /* In the future, we may support decompression, though it is highly doubted
     * whether that is necessary considering the minimalistic nature of our
     * kernel.
     */
    [[nodiscard]] int extractAndMap(BootInformation& bootInformation);
    [[noreturn]] void run(BootInformation& bootInformation);

  private:
    const Elf32_Ehdr*        m_header;
    const Elf32_Phdr* m_programHeaders;
    size_t                                   m_programHeadersCount;

  private:
    const char* m_data;
    size_t m_length;
  };
}
