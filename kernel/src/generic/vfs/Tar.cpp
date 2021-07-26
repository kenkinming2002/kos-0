#include <generic/vfs/Tar.hpp>

#include <librt/Panic.hpp>

namespace core::vfs
{
  namespace
  {
    template<typename T>
    T convertOctal(const char* data, size_t length)
    {
      T result = 0;
      for(size_t i=0; i<length; ++i)
      {
        if(data[i] == '\0')
          break; // We are done

        if(data[i]<'0' || data[i] > '8')
          rt::panic("Corrupted tar file\n");

        result *= 8;
        result += data[i]-'0';
      }
      return result;
    }

    template<typename T, size_t N>
    T convertOctal(const char(&data)[N]) { return convertOctal<T>(data, N); }
  }

  uint32_t TarFileBlock::computeChecksumUnsigned() const
  {
    uint32_t checksum = 0;
    for(size_t i=0; i<BLOCK_SIZE; ++i)
    {
      if(i>=offsetof(TarHeader, checksum) && i<offsetof(TarHeader, checksum)+sizeof tarHeader.checksum)
        checksum += static_cast<unsigned char>(' ');
      else
        checksum += static_cast<unsigned char>(data[i]);
    }
    return checksum;
  }

  uint32_t TarFileBlock::computeChecksumSigned() const
  {
    uint32_t checksum = 0;
    for(size_t i=0; i<BLOCK_SIZE; ++i)
    {
      if(i>=offsetof(TarHeader, checksum) && i<offsetof(TarHeader, checksum)+sizeof tarHeader.checksum)
        checksum += static_cast<signed char>(' ');
      else
        checksum += static_cast<signed char>(data[i]);
    }
    return checksum;
  }

  bool TarFileBlock::isValidHeader() const
  {
    /* We have to compute both signed and unsigned version of the checksum, but
     * in most case, they should be equal. This is because signed and unsigned
     * version of char is bit equivalent for number between 0-127, which is the
     * range of printable char, and the fact that tar make wide use of readable
     * character in its format for ease of manual editing. */
    auto checksum = this->checksum();
    auto unsignedChecksum = computeChecksumUnsigned();
    auto signedChecksum   = computeChecksumSigned();
    return checksum == unsignedChecksum || checksum == signedChecksum;
  }

  bool TarFileBlock::isNullHeader() const { return rt::all(rt::begin(data), rt::end(data), [](char c) { return c == '\0'; }); }
  bool TarFileBlock::isUstarHeader() const { return rt::StringRef(rt::begin(tarHeader.magic), rt::end(tarHeader.magic)) == rt::StringRef("ustar ", 6); }

  bool TarFileBlock::isFile()      const
  {
    return tarHeader.typeflag[0] == TAR_FILE         ||
      tarHeader.typeflag[0] == TAR_REGULAR_FILE ||
      tarHeader.typeflag[0] == TAR_DIRECTORY;
  }

  bool TarFileBlock::isRegularFile() const
  {
    auto fileName = this->fileName();
    return (tarHeader.typeflag[0] == TAR_FILE && fileName[fileName.length()-1] != '/') ||
      tarHeader.typeflag[0] == TAR_REGULAR_FILE;
  }

  bool TarFileBlock::isDirectory() const
  {
    auto fileName = this->fileName();
    return (tarHeader.typeflag[0] == TAR_FILE && fileName[fileName.length()-1] == '/') ||
      tarHeader.typeflag[0] == TAR_DIRECTORY;
  }

  rt::StringRef TarFileBlock::fileName() const
  {
    auto length = rt::strnlen(tarHeader.name, sizeof tarHeader.name);
    return rt::StringRef(tarHeader.name, length);
  }
  const char* TarFileBlock::fileData() const { return reinterpret_cast<const char*>(nextBlock()); }
  size_t TarFileBlock::fileSize() const { return convertOctal<size_t>(tarHeader.size); }
  uint32_t TarFileBlock::checksum() const { return convertOctal<uint32_t>(tarHeader.checksum); }

  const TarFileBlock* TarFileBlock::nextBlock() const { return this+1; }
  const TarFileBlock* TarFileBlock::nextHeaderBlock() const
  {
    auto fileBlocksCount = (fileSize()+(BLOCK_SIZE-1)) / BLOCK_SIZE;
    return nextBlock()+fileBlocksCount;
  }
}
