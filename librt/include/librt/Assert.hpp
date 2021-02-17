#pragma once

namespace rt::internals
{
  void assertFunc(const char* file, int line, const char* func, const char* expr);
}

#define ASSERT(expr) do { if(!(expr)) rt::internals::assertFunc(__FILE__, __LINE__, __PRETTY_FUNCTION__, #expr); } while(0)
