#include <core/init/MemoryMap.hpp>

struct multiboot_tag_mmap *multiboot_mmap_tag;

int multiboot2_tag_mmap_parse(struct multiboot_tag_mmap *mmap_tag)
{
  multiboot_mmap_tag = mmap_tag;
}
