#include <generic/memory/Memory.hpp>

void *operator new(size_t size)
{
  return core::memory::malloc(size);
}

void *operator new[](size_t size)
{
  return core::memory::malloc(size);
}

void operator delete(void *p)
{
  core::memory::free(p);
}

void operator delete[](void *p)
{
  core::memory::free(p);
}

void operator delete(void *p, size_t /*sz*/)
{
  core::memory::free(p);
}

void operator delete[](void *p, size_t /*sz*/)
{
  core::memory::free(p);
}
