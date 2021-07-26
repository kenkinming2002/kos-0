#include <i686/syscalls/Access.hpp>

#include <limits>

namespace core::syscalls
{
  namespace
  {
    bool verifyUserBuffer(const char* buffer, size_t length)
    {
      /* We only do prelimary check, and leave the rest to be handled via page
       * fault */
      return reinterpret_cast<uintptr_t>(buffer) < std::numeric_limits<uintptr_t>::max()-length &&
             reinterpret_cast<uintptr_t>(buffer)+length<0xC0000000;
    }

    Result<size_t> strlenFromUser(const char* str)
    {
      for(const char* it = str;;++it)
      {
        char c;
        if(auto result = InputUserBuffer(it, 1).read(c); !result)
          return result.error();

        if(c == '\0')
          return it - str;
      }
    }
  }

  Result<InputUserBuffer> InputUserBuffer::fromCString(const char* str)
  {
      auto length = strlenFromUser(str);
      if(!length)
        return length.error();

      return InputUserBuffer(str, *length+1);
  }

  Result<void> InputUserBuffer::read(char* buf, size_t length)
  {
    if(m_length < length)
      return ErrorCode::INVALID;

    if(!verifyUserBuffer(m_buf, length))
      return ErrorCode::FAULT;

    /* TODO: Gracefully handle page fault. In linux, this is handled by the
     * __might_fault() function, which save the address after the instruction
     * that might fault in a lookup table and allow the page fault handler to
     * resume execution. */
    rt::copy(m_buf, m_buf+length, buf);
    m_buf    += length;
    m_length -= length;
    return {};
  }

  Result<void> OutputUserBuffer::write(const char* buf, size_t length)
  {
    if(m_length < length)
      return ErrorCode::INVALID;

    if(!verifyUserBuffer(m_buf, length))
      return ErrorCode::FAULT;

    /* TODO: Gracefully handle page fault. In linux, this is handled by the
     * __might_fault() function, which save the address after the instruction
     * that might fault in a lookup table and allow the page fault handler to
     * resume execution. */
    rt::copy(buf, buf+length, m_buf);
    m_buf    += length;
    m_length -= length;

    return {};
  }
}
