#pragma once

#include <core/init/multiboot2.h>

extern struct multiboot_tag_mmap *multiboot_mmap_tag;
int multiboot2_tag_mmap_parse(struct multiboot_tag_mmap *mmap_tag);
