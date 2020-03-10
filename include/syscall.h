#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int16_t tid_t;

int syscall_0(unsigned syscall_number);
int syscall_1(unsigned syscall_number, unsigned a1);
int syscall_2(unsigned syscall_number, unsigned a1, unsigned a2);
int syscall_3(unsigned syscall_number, unsigned a1, unsigned a2, unsigned a3);
int syscall_4(unsigned syscall_number, unsigned a1, unsigned a2, unsigned a3, unsigned a4);


inline int write(const char* buf, size_t count)
{
  return syscall_2(0x8, reinterpret_cast<uintptr_t>(buf), count);
}

inline int yield()
{
  return syscall_0(0x0);
}

inline int create_service(const char* name)
{
  return syscall_1(0x1, reinterpret_cast<uintptr_t>(name));
}

inline int destroy_service(int handle)
{
  return syscall_1(0x2, reinterpret_cast<unsigned&>(handle));
}

inline int locate_service(const char* name)
{
  return syscall_1(0x3, reinterpret_cast<uintptr_t>(name));
}

inline int send(tid_t dst, const char* buf, size_t count)
{
  return syscall_3(0x4, dst, reinterpret_cast<uintptr_t>(buf), count);
}

inline int recv(tid_t src, const char* buf, size_t count)
{
  return syscall_3(0x5, src, reinterpret_cast<uintptr_t>(buf), count);
}

#ifdef __cplusplus
}
#endif
