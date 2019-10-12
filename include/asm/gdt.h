#ifndef ASM_GDT_H
#define ASM_GDT_H

#ifdef __cplusplus
extern "C" {
#endif

/** lgdt - Loads the global descriptor table
 *  @param gdt The pointer to global descriptor table
 */
void lgdt(const void* gdt);

/** gdt_flush - Load the segment register
 */
void gdt_flush();

#ifdef __cplusplus
}
#endif

#endif // ASM_GDT_H
