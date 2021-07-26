#pragma once

namespace rt::internals
{
  [[noreturn]] void assertFunc(const char* file, int line, const char* func, const char* expr);
}

#define _ASSERT(expr) do { if(!(expr)) rt::internals::assertFunc(__FILE__, __LINE__, __PRETTY_FUNCTION__, #expr); } while(0)

#ifndef NDEBUG
#define ASSERT(expr) _ASSERT(expr)
#else
#define ASSERT(expr)
#endif

#ifndef NDEBUG
#define ASSERT_ALWAYS(expr) _ASSERT(expr)
#else
#define ASSERT_ALWAYS(expr) (void)(expr)
#endif

#ifndef NDEBUG
#define ASSERT_UNREACHABLE _ASSERT(false && "Unreachable Code")
#else
#define ASSERT_UNREACHABLE do { __builtin_unreachable(); } while(0)
#endif

#define ASSERT_UNIMPLEMENTED _ASSERT(false && "Unimplemented function")

#define ASSERT_FALSE(expr) ASSERT(!(expr))
#define ASSERT_TRUE(expr) ASSERT(expr)
