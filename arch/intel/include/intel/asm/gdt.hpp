#pragma once

namespace assembly
{
  /** lgdt - Loads the global descriptor table
   *  @param gdt The pointer to global descriptor table
   */
  void lgdt(const void* gdt);

  /** gdt_flush - Load the segment register
   */
  void gdt_flush();
}
