#include <librt/Hooks.hpp>

#include <new>

void *operator new(size_t size)
{
  return rt::hooks::malloc(size);
}

void *operator new[](size_t size)
{
  return rt::hooks::malloc(size);
}

void operator delete(void *p)
{
  rt::hooks::free(p);
}

void operator delete[](void *p)
{
  rt::hooks::free(p);
}

void operator delete(void *p, size_t /*sz*/)
{
  rt::hooks::free(p);
}

void operator delete[](void *p, size_t /*sz*/)
{
  rt::hooks::free(p);
}

