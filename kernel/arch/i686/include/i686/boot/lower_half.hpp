#pragma once

#include <i686/boot/boot.hpp>

extern "C" BOOT_FUNCTION [[gnu::noinline]] void lower_half_main(std::byte* tag);
