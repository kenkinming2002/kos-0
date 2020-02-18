#include <generic/io/Print.hpp>

#include <generic/io/Framebuffer.hpp>
#include <generic/io/Serial.hpp>

#include <type_traits>

extern "C" size_t strlen(const char* str);

namespace io
{
  static constexpr size_t TO_STRING_BUF_SIZE = 32;
  static char toStringBuf[TO_STRING_BUF_SIZE];

  /***********************************
   * Integer Formatting and Printing *
   ***********************************/
  template int printSingle<uint16_t, Base::HEX>(uint16_t);
  template int printSingle<uint32_t, Base::HEX>(uint32_t);
  template int printSingle<uint64_t, Base::HEX>(uint64_t);

  template int printSingle<int16_t, Base::HEX>(int16_t);
  template int printSingle<int32_t, Base::HEX>(int32_t);
  template int printSingle<int64_t, Base::HEX>(int64_t);

  char toAscii(unsigned placeValue, Base base)
  {
    switch(base)
    {
    case Base::BIN:
      return "01"[placeValue];
    case Base::DEC:
      return "0123456789"[placeValue];
    case Base::OCT:
      return "012345678"[placeValue];
    case Base::HEX:
      return "0123456789ABCDEF"[placeValue];
    }

    return ' ';
  }

  template<typename T, Base base>
  std::enable_if_t<std::is_unsigned_v<T>, char*> toString_impl(T value)
  {
    toStringBuf[TO_STRING_BUF_SIZE-1] = '\0';
    for(size_t i=TO_STRING_BUF_SIZE-2; i>=0; --i)
    {
      T placeValue = value % static_cast<T>(base);
      toStringBuf[i] = toAscii(placeValue, base);

      value /= static_cast<T>(base); if(value==0) return &toStringBuf[i]; }

    return toStringBuf; // Overflow, return whatever is written
  }

  template<typename T, Base base>
  std::enable_if_t<std::is_signed_v<T>, char*> toString_impl(T value)
  {
    std::make_unsigned_t<T> unsignedValue = value; // Unsigned value have twice the range of its signed counterpart so this is safe
    char* unsignedString = toString_impl<std::make_unsigned_t<T>, base>(unsignedValue);
    if(value<0)
      *(--unsignedString) = '-';

    return unsignedString;
  }

  template<typename T, Base base>
  const char* toString(T value)
  {
    return toString_impl<T, base>(value);
  }

  template<typename T, Base base>
  int printSingle(T value)
  {
    const char* str = toString<T, base>(value);
    size_t len = &toStringBuf[TO_STRING_BUF_SIZE-1] - str;

    io::frameBuffer.write(str, len);
    io::com1Port.write(str, len);

    return len;
  }

  /*******************
   * String Printing *
   *******************/
  int printSingle(const char* str)
  {
    size_t len = strlen(str);
    return printSingle(str, len);
  }

  int printSingle(const char* str, size_t len)
  {
    io::frameBuffer.write(str, len);
    io::com1Port.write(str, len);

    return len;
  }
}
