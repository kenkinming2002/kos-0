#pragma once

#include <librt/Utility.hpp>
#include <librt/StringRef.hpp>
#include <librt/String.hpp>

#include <generic/Error.hpp>

#include <sys/Types.hpp>

#include <type_traits>

#include <stddef.h>

namespace core::syscalls
{
  struct InputUserBuffer
  {
  public:
    static Result<InputUserBuffer> fromCString(const char* str);

  public:
    constexpr InputUserBuffer(const char* buf, size_t length) : m_buf(buf), m_length(length) {}

  public:
    Result<void> read(char* buf, size_t length);
    template<typename T>
    Result<void> read(T& t) requires std::is_standard_layout_v<T> { return read(reinterpret_cast<char*>(&t), sizeof t); }
    Result<void> read(rt::String& string)
    {
      if(m_length == 0)
        return ErrorCode::INVALID;

      string = rt::String(m_length-1, '?'); // The last char is a null-terminator
      if(auto result = read(string.data(), string.length()); !result)
        return result;

      ASSERT(m_length == 1);
      m_length = 0;

      return {};
    }

  private:
    const char* m_buf;
    size_t      m_length;
  };

  struct OutputUserBuffer
  {
  public:
    constexpr OutputUserBuffer(char* buf, size_t length) : m_buf(buf), m_length(length) {}

  public:
    size_t length() const { return m_length; }

  public:
    Result<void> write(const char* buf, size_t length);

  public:
    template<typename T>
    Result<void> write(const T& t) requires std::is_trivial_v<T> { return write(reinterpret_cast<const char*>(&t), sizeof t); }
    Result<void> write(rt::StringRef string)
    {
      if(auto result = write(string.data(), string.length()); !result)
        return result;

      if(auto result = write('\0'); !result)
        return result;

      return {};
    }

  public:
    template<typename... Args>
    [[gnu::noinline]] Result<void> writeAll(Args&&... args)
    {
      Result<void> result;

      auto save = *this;
      auto helper = [&](auto&& arg){
        if(!result)
          return;

        result = write(rt::forward<decltype(arg)>(arg));
      };
      (helper(rt::forward<Args>(args)), ...);

      if(!result)
        *this = save; // Roll back

      return result;
    }

  private:
    char*  m_buf;
    size_t m_length;
  };

  inline Result<rt::String> stringFromUser(const char* str)
  {
    auto userBuffer = InputUserBuffer::fromCString(str);
    ASSERT(userBuffer);
    if(!userBuffer)
      return userBuffer.error();

    rt::String string;
    auto result = userBuffer->read(string);
    ASSERT(result);
    if(!result)
      return result.error();

    return rt::move(string);
  }

  inline bool verifyRegionUser(uintptr_t addr, size_t length)
  {
    uintptr_t end;
    if(__builtin_add_overflow(addr, length, &end))
      return false;
    return end<=0xC0000000;
  }
}
