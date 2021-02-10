#include <boot/generic/Kernel.hpp>

#include <boot/generic/multiboot2-Utils.hpp>
#include <boot/generic/Config.h>
#include <boot/i686/Paging.hpp>
#include <librt/Panic.hpp>

#include <librt/Iterator.hpp>

#include <librt/Strings.hpp>
#include <librt/Algorithm.hpp>

namespace boot
{
  [[gnu::section(".kernelImage")]] alignas(0x1000) char kernelImageStorage[KERNEL_IMAGE_STORAGE_SIZE];
  char* kernelImageEnd = kernelImageStorage;

  rt::Optional<Kernel> Kernel::from(const multiboot_boot_information* multiboot2BootInformation)
  {  
    using namespace common::tasks;
    
    Kernel kernel;
    for(auto* tag = multiboot2BootInformation->tags; tag->type != MULTIBOOT_TAG_TYPE_END; tag = multiboot2::next_tag(tag))
      if(tag->type == MULTIBOOT_TAG_TYPE_MODULE)
      {
        auto* module_tag = reinterpret_cast<const struct multiboot_tag_module*>(tag);
        if(rt::strncmp(module_tag->cmdline, "kernel", 6) == 0)
        {
          kernel.m_data   = reinterpret_cast<char*>(module_tag->mod_start);
          kernel.m_length = module_tag->mod_end-module_tag->mod_start;
          kernel.m_header = getElf32Header(kernel.m_data, kernel.m_length);
          if(!kernel.m_header)
            return rt::nullOptional;

          kernel.m_programHeaders = getElf32ProgramHeaders(kernel.m_data, kernel.m_length, kernel.m_header, kernel.m_programHeadersCount);
          if(!kernel.m_programHeaders)
            return rt::nullOptional;

          return kernel;
        }
      }

    return rt::nullOptional;
  }

  int Kernel::extract(size_t loadOffset)
  {
    for(size_t i=0; i<m_programHeadersCount; ++i)
    {
      const auto& programHeader = m_programHeaders[i];
      char* segmentBegin = kernelImageStorage + (programHeader.vaddr - loadOffset);
      char* segmentEnd   = kernelImageStorage + (programHeader.vaddr - loadOffset) + programHeader.memsz;

      const char* fileSegmentBegin = m_data+programHeader.offset;
      const char* fileSegmentEnd   = m_data+programHeader.offset+programHeader.filesz;

      if(programHeader.vaddr<loadOffset)
        return -1;

      if(segmentEnd>rt::end(kernelImageStorage))
        return -1;

      if(segmentEnd>kernelImageEnd)
        kernelImageEnd = segmentEnd;

      if(programHeader.filesz>programHeader.memsz)
        return -1;

      rt::fill(segmentBegin, segmentEnd, 0);
      rt::copy(fileSegmentBegin, fileSegmentEnd, segmentBegin);
    }

    return 0;
  }

  int Kernel::map(size_t loadOffset)
  {
    using namespace boot::memory;
    using namespace common::memory;

    for(size_t i=0; i<m_programHeadersCount; ++i)
    {
      const auto& programHeader = m_programHeaders[i];
      char* segmentBegin = kernelImageStorage + (programHeader.vaddr - loadOffset);
      if(int result = memory::map(reinterpret_cast<uintptr_t>(segmentBegin), programHeader.vaddr, programHeader.memsz, Access::SUPERVISOR_ONLY, Permission::READ_WRITE); result != 0)
        return result;
    }

    return 0;
  }

  void Kernel::run(BootInformation& bootInformation)
  {
    using namespace boot::memory;
    using namespace common::memory;

    asm volatile ( 
        // Load page directory
        "jmp %[entry];"       
        : : [entry]"r"(m_header->entry), "b"(&bootInformation) : "eax", "memory"
        );
    __builtin_unreachable();
  }
  
  Kernel kernel;
  int initKernel(const multiboot_boot_information* multiboot2BootInformation)
  {
    auto result = Kernel::from(multiboot2BootInformation);
    if(!result)
      return -1;

    kernel = *result;

    if(auto result = kernel.extract(0xC0000000); result != 0)
      return result;

    if(auto result = kernel.map(0xC0000000); result != 0)
      return result;

    return 0;
  }

  void updateBootInformationKernel()
  {
    addMemoryRegion(reinterpret_cast<uintptr_t>(kernelImageStorage), reinterpret_cast<uintptr_t>(kernelImageEnd) - reinterpret_cast<uintptr_t>(kernelImageStorage), MemoryRegion::Type::KERNEL);
  }

  [[noreturn]] void runKernel(BootInformation& bootInformation)
  {
    kernel.run(bootInformation);
  }
}

