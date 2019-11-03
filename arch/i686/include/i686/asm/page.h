#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/** enable_paging - Enable paging
 *
 * @param page_directory  pointer to page directory structure
 */
void enable_paging(void* page_directory);

/** flush_tlb - Flush the Trnaslation Lookaside Buffer
 */
void flush_tlb();

#ifdef __cplusplus
}
#endif
