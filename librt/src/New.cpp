#include <librt/Hooks.hpp>

#include <liballoc_1_1.h>

#include <new>

void* malloc(size_t size) { return ::kmalloc(size); }
void* realloc(void* ptr, size_t size) { return ::krealloc(ptr, size); }
void* calloc(size_t nmemb, size_t size) { return ::kcalloc(nmemb, size); }
void free(void* ptr) { return ::kfree(ptr); }

void *operator new(size_t size)
{
  return ::malloc(size);
}

void *operator new[](size_t size)
{
  return ::malloc(size);
}

void operator delete(void *p)
{
  ::free(p);
}

void operator delete[](void *p)
{
  ::free(p);
}

void operator delete(void *p, size_t /*sz*/)
{
  ::free(p);
}

void operator delete[](void *p, size_t /*sz*/)
{
  ::free(p);
}

