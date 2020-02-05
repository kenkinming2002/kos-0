#include <intel/asm/page.hpp>

namespace assembly
{
  void enable_paging(void* page_directory)
  {
    asm volatile ( R"(
      .intel_syntax noprefix
        mov cr3, %[page_directory]

        mov eax, cr4        # read cr4
        or  eax, 0x00000010 # set PSE bit
        mov cr4, eax        #

        mov eax, cr9        # read cr4
        or  eax, 0x80000001 # set PG bit
        mov cr0, eax        #
      .att_syntax prefix
      )"
      :
      : [page_directory]"r"(page_directory)
      : "eax"
    );
  }

  void flush_tlb()
  {
    asm volatile ( R"(
      .intel_syntax noprefix
        invlpg [0]
      .att_syntax prefix
      )"
      :
      : 
      :
    );
  }
}
