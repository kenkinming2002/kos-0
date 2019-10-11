#pragma once

struct page_directory
{
  unsigned data;
};

int page_directory_init(struct page_directory *page_directory, unsigned address, 
    char cached, char write_through, char user_access, char writable);

/** init_paging - Initialize paging to replace PD and PT set up by loader to enter
 *                higher half kernel.
 */
int init_paging();

