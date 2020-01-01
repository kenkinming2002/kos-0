#pragma once

/**
 * @file include/i686/boot/boot.hpp
 *
 * Basics macro for accessing data and functions at boot time before higher-half
 * kernel is set-up.
 */

#define BOOT_FUNCTION [[gnu::section(".boot.text")]]

// Obtain address of data before setting up of paging (i.e. convert virtual
// address to physical address)
#define BOOT_ADDRESS
