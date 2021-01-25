#pragma once

#include <boot/generic/BootInformation.hpp>
#include <boot/generic/multiboot2.h>
#include <common/generic/tasks/Elf.hpp>

#include <optional>

#include <stddef.h>

namespace boot
{
  class Kernel
  {
  public:
    static std::optional<Kernel> from(const multiboot_boot_information* multiboot2BootInformation);

  public:
    /* In the future, we may support decompression, though it is highly doubted
     * whether that is necessary considering the minimalistic nature of our
     * kernel.
     */
    [[nodiscard]] int extract(size_t loadOffset);
    [[nodiscard]] int map(size_t loadOffset);
    [[noreturn]] void run(BootInformation& bootInformation);

  private:
    const common::tasks::Elf32Header*        m_header;
    const common::tasks::ELF32ProgramHeader* m_programHeaders;
    size_t                                   m_programHeadersCount;

  private:
    const char* m_data;
    size_t m_length;
  };

  int initKernel(const multiboot_boot_information* multiboot2BootInformation);
  void updateBootInformationKernel();
  [[noreturn]] void runKernel(BootInformation& bootInformation);
}
