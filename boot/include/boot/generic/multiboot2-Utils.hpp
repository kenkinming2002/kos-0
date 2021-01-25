#pragma once

#include <boot/generic/multiboot2.h>

#include <stdint.h>

namespace multiboot2
{
  inline struct multiboot_tag* next_tag(struct multiboot_tag* tag) { return reinterpret_cast<struct multiboot_tag*>(reinterpret_cast<uintptr_t>(tag) + ((tag->size+7)&~7)); }

  inline struct multiboot_mmap_entry* mmap_entry_begin(struct multiboot_tag_mmap* mmap_tag) { return mmap_tag->entries; }
  inline struct multiboot_mmap_entry* mmap_entry_end(struct multiboot_tag_mmap* mmap_tag) { return reinterpret_cast<struct multiboot_mmap_entry*>(reinterpret_cast<uintptr_t>(mmap_tag->entries) + mmap_tag->size - sizeof mmap_tag); }
  inline struct multiboot_mmap_entry* next_mmap_entry(struct multiboot_tag_mmap* mmap_tag, struct multiboot_mmap_entry* mmap_entry) { return reinterpret_cast<struct multiboot_mmap_entry*>(reinterpret_cast<uintptr_t>(mmap_entry)+mmap_tag->entry_size); }

  inline const struct multiboot_tag* next_tag(const struct multiboot_tag* tag) { return reinterpret_cast<struct multiboot_tag*>(reinterpret_cast<uintptr_t>(tag) + ((tag->size+7)&~7)); }

  inline const struct multiboot_mmap_entry* mmap_entry_begin(const struct multiboot_tag_mmap* mmap_tag) { return mmap_tag->entries; }
  inline const struct multiboot_mmap_entry* mmap_entry_end(const struct multiboot_tag_mmap* mmap_tag) { return reinterpret_cast<const struct multiboot_mmap_entry*>(reinterpret_cast<uintptr_t>(mmap_tag->entries) + mmap_tag->size - sizeof mmap_tag); }
  inline const struct multiboot_mmap_entry* next_mmap_entry(const struct multiboot_tag_mmap* mmap_tag, const struct multiboot_mmap_entry* mmap_entry) { return reinterpret_cast<const struct multiboot_mmap_entry*>(reinterpret_cast<uintptr_t>(mmap_entry)+mmap_tag->entry_size); }
}
