#pragma once

#include <boot/i686/Boot.hpp>

extern "C" BOOT_FUNCTION [[gnu::noinline]] void lower_half_main(char* tag);
