#pragma once

#include <sys/Types.hpp>

#include <generic/Error.hpp>
#include <generic/vfs/SuperBlock.hpp>

#include <librt/Utility.hpp>
#include <librt/Optional.hpp>
#include <librt/StringRef.hpp>
#include <librt/UniquePtr.hpp>
#include <librt/SharedPtr.hpp>
#include <librt/Pair.hpp>
#include <librt/Result.hpp>
#include <librt/String.hpp>

#include <type_traits>

namespace core::vfs
{
  struct Dirent
  {
    uword_t length; // This is needed to maintain forward compatiblility - length of the entire structure
    uword_t ino;
    uword_t type;

    char name[];
  };

  /*
   * Inode is not always associated with a unique Vnode, but that must be the
   * case for a directory inode, since there is no directory hardlink.
   */
  class Inode : public rt::SharedPtrHook
  {
  public:
    virtual ~Inode() = default;

  /*********************
   * General interface *
   *********************/
  private:
    static inline constinit SuperBlock m_superBlock;

  public:
    virtual const SuperBlock& superBlock() const { return m_superBlock; }
    virtual SuperBlock& superBlock()             { return m_superBlock; }

  public:
    struct Stat
    {
      dev_t dev = UNKNWON_DEV; /* device number */
      ino_t ino = UNKNWON_INO; /* Inode number */

      Type   type;
      size_t blockSize; /* This must be a power of 2 */
      size_t size;

      /* for debugging purposes */
      const char* moduleName = "unknown";
      const char* deviceName = "unknown";
    };
    virtual Result<Stat> stat() { return ErrorCode::UNSUPPORTED; }

  /******************
   * File interface *
   ******************/
  public:
    virtual Result<ssize_t> read(char* buf, size_t length, size_t pos)        { return ErrorCode::UNSUPPORTED; }
    virtual Result<ssize_t> write(const char* buf, size_t length, size_t pos) { return ErrorCode::UNSUPPORTED; }
    virtual Result<void>   resize(size_t size)                                { return ErrorCode::UNSUPPORTED; }

  public:
    virtual Result<ssize_t> readdir(char* buf, size_t length) { return ErrorCode::UNSUPPORTED; }

  /*******************
   * Vnode interface *
   *******************/
  public:
    virtual Result<rt::SharedPtr<Inode>> lookup(rt::StringRef name) { return ErrorCode::UNSUPPORTED; }

  public:
    virtual Result<rt::SharedPtr<Inode>> create(rt::StringRef name, Type type) { return ErrorCode::UNSUPPORTED; }
    virtual Result<void> link(rt::StringRef name, rt::SharedPtr<Inode> inode)  { return ErrorCode::UNSUPPORTED; }
    virtual Result<void> unlink(rt::StringRef name)                            { return ErrorCode::UNSUPPORTED; }
  };
}
