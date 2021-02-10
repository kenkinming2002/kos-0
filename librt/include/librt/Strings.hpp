#pragma once

#include <stdint.h>
#include <stddef.h>

namespace rt
{
  void *memset(void *s, int c, size_t n);
  void *memcpy(void *dst, const void *src, size_t n);
  void *memmove(void *dst, const void *src, size_t n);
  void *memchr(const void *buf, int c, size_t n);
  int memcmp(const void *s1, const void *s2, size_t n);
  size_t strlen(const char *str);
  size_t strnlen(const char *str, size_t maxlen);
  char *strcpy(char *dst, const char *src);
  char *strncpy(char *dst, const char *src, size_t n);
  int strcmp(const char *s1, const char *s2);
  int strncmp(const char *s1, const char *s2, size_t num);
  char *strcpy_safe(char *dst, const char *src, size_t dstSize);
}
