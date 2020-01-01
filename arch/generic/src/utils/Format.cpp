#include <generic/utils/Format.hpp>

namespace utils
{
  namespace
  {
    constexpr auto ch = [](auto digit) {
      if(digit>=0 && digit<10)
        return '0' + digit;
      else
        return 'A' + (digit-10);
    };
  }

  string_type format(uint16_t i)
  {
    static char buf[4];
    buf[0] = ch((i >> 12) & 0xF);
    buf[1] = ch((i >>  8) & 0xF);
    buf[2] = ch((i >>  4) & 0xF);
    buf[3] = ch((i >>  0) & 0xF);

    return buf;
  }

  string_type format(uint32_t i)
  {
    static char buf[8];
    buf[0] = ch((i >> 28) & 0xF);
    buf[1] = ch((i >> 24) & 0xF);
    buf[2] = ch((i >> 20) & 0xF);
    buf[3] = ch((i >> 16) & 0xF);
    buf[4] = ch((i >> 12) & 0xF);
    buf[5] = ch((i >>  8) & 0xF);
    buf[6] = ch((i >>  4) & 0xF);
    buf[7] = ch((i >>  0) & 0xF);

    return buf;
  }

  string_type format(uint64_t i)
  {
    static char buf[16];
    buf[0]  = ch((i >> 60) & 0xF);
    buf[1]  = ch((i >> 56) & 0xF);
    buf[2]  = ch((i >> 52) & 0xF);
    buf[3]  = ch((i >> 48) & 0xF);
    buf[4]  = ch((i >> 44) & 0xF);
    buf[5]  = ch((i >> 40) & 0xF);
    buf[6]  = ch((i >> 36) & 0xF);
    buf[7]  = ch((i >> 32) & 0xF);
    buf[8]  = ch((i >> 28) & 0xF);
    buf[9]  = ch((i >> 24) & 0xF);
    buf[10] = ch((i >> 20) & 0xF);
    buf[11] = ch((i >> 16) & 0xF);
    buf[12] = ch((i >> 12) & 0xF);
    buf[13] = ch((i >>  8) & 0xF);
    buf[14] = ch((i >>  4) & 0xF);
    buf[15] = ch((i >>  0) & 0xF);

    return buf;
  }


  int format(char* buf, size_t len, uint16_t val)
  {
    constexpr static size_t STRING_LENGTH = 4;
    if(buf==nullptr && len == 0)
      return STRING_LENGTH;
    if(len < STRING_LENGTH)
      return -1;

    for(size_t i=0; i<STRING_LENGTH; ++i)
    {
      unsigned shift = 4 * (STRING_LENGTH-i-1);
      buf[i] = ch((val >> shift) & 0xF);
    }
    return STRING_LENGTH;
  }

  int format(char* buf, size_t len, uint32_t val)
  {
    constexpr static size_t STRING_LENGTH = 8;
    if(buf==nullptr && len == 0)
      return STRING_LENGTH;
    if(len < STRING_LENGTH)
      return -1;

    for(size_t i=0; i<STRING_LENGTH; ++i)
    {
      unsigned shift = 4 * (STRING_LENGTH-i-1);
      buf[i] = ch((val >> shift) & 0xF);
    }
    return STRING_LENGTH;
  }

  int format(char* buf, size_t len, uint64_t val)
  {
    constexpr static size_t STRING_LENGTH = 16;
    if(buf==nullptr && len == 0)
      return STRING_LENGTH;
    if(len < STRING_LENGTH)
      return -1;

    for(size_t i=0; i<STRING_LENGTH; ++i)
    {
      unsigned shift = 4 * (STRING_LENGTH-i-1);
      buf[i] = ch((val >> shift) & 0xF);
    }
    return STRING_LENGTH;
  }

  int format(char* buf, size_t len, const char* str)
  {
    // NOTE: There's possiblity of integer overflow if len exceed the maximum
    //       value representable by int64_t
    int64_t capacity = len;
    size_t i;
    while(*str)
    {
      *buf++ = *str++;
      if(--capacity == 0)
        return len;
    }
    return len - capacity;
  }
}
