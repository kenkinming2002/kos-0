# Hobbyist OS (There is no name for the OS as of now)
This is an implementation of a microkernel, which aims to implement as little
thing as possible in the kernel but instead relies on userspace support.

# Prerequsite
 - Meson build system
 - i686-elf gcc cross compiler(currently clang is not supported yet, as the OS
   depends on feature from the standard libary such as std::optional, which
   should be replaced soon)

# Building
