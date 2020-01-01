#pragma once

constexpr unsigned BOOT_PAGE_DIRECTORY_SIZE = 4096;
constexpr unsigned BOOT_PAGE_TABLE_SIZE     = 4096;
constexpr unsigned BOOT_PAGE_TABLE_COUNT    = 1;

extern "C"
{
  extern char boot_page_directory[BOOT_PAGE_DIRECTORY_SIZE];
  extern char boot_page_table[BOOT_PAGE_TABLE_SIZE * BOOT_PAGE_TABLE_COUNT];
  extern char boot_page_tables[BOOT_PAGE_TABLE_SIZE * BOOT_PAGE_TABLE_COUNT];
}
