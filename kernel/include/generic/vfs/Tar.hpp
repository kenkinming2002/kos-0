#pragma once

#include <librt/StringRef.hpp>
#include <librt/Panic.hpp>

#include <stdint.h>
#include <stddef.h>

namespace core::vfs
{
  // Unix-Standard Tar Header
  struct [[gnu::packed]] TarHeader
  {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag[1];
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
  };

  enum TarTypeFlag
  {
    TAR_FILE                  = '\0', /* Used by old style archive */
    TAR_REGULAR_FILE          = '0',
    TAR_HARD_LINK             = '1',
    TAR_SYMBOLIC_LINK         = '2',
    TAR_CHARACTER_DEVICE_NODE = '3',
    TAR_BLOCK_DEVICE_NODE     = '4',
    TAR_DIRECTORY             = '5',
    TAR_FIFO_NODE             = '6',
    TAR_RESERVED              = '7'
  };


  union TarFileBlock
  {
  public:
    static constexpr size_t BLOCK_SIZE = 512;

  public:
    TarHeader tarHeader;
    char data[BLOCK_SIZE];

  public:
    uint32_t computeChecksumUnsigned() const;
    uint32_t computeChecksumSigned() const;
    bool isValidHeader() const;

  public:
    bool isNullHeader() const;
    bool isUstarHeader() const;

    bool isFile()      const;
    bool isRegularFile() const;
    bool isDirectory() const;

  public:
    rt::StringRef fileName() const;
    const char* fileData() const;
    size_t fileSize() const;
    uint32_t checksum() const;

  public:
    const TarFileBlock* nextBlock() const;
    const TarFileBlock* nextHeaderBlock() const;
  };
}

