#ifndef ASM_GDT_H
#define ASM_GDT_H

/** lgdt - Loads the global descriptor table
 *  @param gdt The pointer to global descriptor table
 */
void lgdt(void* gdt);

/** gdt_flush - Load the segment register
 */
void gdt_flush();

#endif // ASM_GDT_H
