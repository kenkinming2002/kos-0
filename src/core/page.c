#include <core/page.h>

int page_directory_init(struct page_directory *page_directory, unsigned address, 
    char cached, char write_through, char user_access, char writable)
{
  page_directory->data = 0x00000000;

  page_directory->data |= address & 0xFFFFF000;
  //page_directory->data |= 1<<7; // 4 MiB page
  if(!cached)
    page_directory->data |= 1<<4;
  if(write_through)
    page_directory->data |= 1<<3;
  if(user_access)
    page_directory->data |= 1<<2;
  if(writable)
    page_directory->data |= 1<<1;

  page_directory->data |= 1; // Present
}
