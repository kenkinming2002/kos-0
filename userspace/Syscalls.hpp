#pragma once

#include <stdint.h>
#include <stddef.h>

#include <limits>

#include <librt/Cast.hpp>
#include <librt/Utility.hpp>

// Ideally, this part would be found in kernel header
using word_t  = intptr_t;
using uword_t = uintptr_t;

using size_t  = uword_t;
using ssize_t = word_t;
using off_t   = word_t;

using dev_t = uword_t;
using ino_t = uword_t;

using result_t = word_t;
using fd_t     = word_t;

static constexpr dev_t UNKNWON_DEV = std::numeric_limits<dev_t>::max();
static constexpr ino_t UNKNWON_INO = std::numeric_limits<ino_t>::max();

enum class Type   : uword_t { REGULAR_FILE, DIRECTORY, SYMBOLIC_LINK, OTHER };
enum class Anchor : uword_t { BEGIN, CURRENT , END };

enum Syscalls : uword_t
{
  SYS_YIELD    = 0,
  SYS_ROOT     = 1,
  SYS_MOUNTAT  = 2,
  SYS_UMOUNTAT = 3,
  SYS_OPENAT   = 4,
  SYS_CREATEAT = 5,
  SYS_LINKAT   = 6,
  SYS_UNLINKAT = 7,
  SYS_READDIR  = 8,
  SYS_SEEK     = 9,
  SYS_READ     = 10,
  SYS_WRITE    = 11,
  SYS_RESIZE   = 12,
  SYS_CLOSE    = 13,

  SYS_TEST     = 14,
  SYS_LOG      = 15
};

extern "C" uword_t syscall(uword_t syscallNumber, uword_t a1, uword_t a2, uword_t a3, uword_t a4, uword_t a5, uword_t a6);

template<typename... Args>
inline result_t syscallWrapper(Args&&... args)
{
  uword_t result = syscall(rt::bitCast<uword_t>(rt::forward<Args>(args))...);
  return rt::bitCast<result_t>(result);
}

inline result_t sys_root()                                                                                 { return syscallWrapper(SYS_ROOT, uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }

inline result_t sys_mountAt(fd_t atfd, const char* mountpoint, const char* mountableName, const char* arg) { return syscallWrapper(SYS_MOUNTAT,  atfd, mountpoint, mountableName, arg,        uword_t(0), uword_t(0)); }
inline result_t sys_umountAt(fd_t atfd, const char* mountpoint)                                            { return syscallWrapper(SYS_UMOUNTAT, atfd, mountpoint, uword_t(0),    uword_t(0), uword_t(0), uword_t(0)); }

inline fd_t sys_openAt(fd_t atfd, const char* path)                     { return syscallWrapper(SYS_OPENAT,   atfd, path, uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }
inline fd_t sys_createAt(fd_t atfd, const char* path, Type type)        { return syscallWrapper(SYS_CREATEAT, atfd, path, type,       uword_t(0), uword_t(0), uword_t(0)); }
inline result_t sys_linkAt(fd_t atfd, const char* path, const char* target) { return syscallWrapper(SYS_LINKAT,   atfd, path, target,     uword_t(0), uword_t(0), uword_t(0)); }
inline result_t sys_unlinkAt(fd_t atfd, const char* path)                   { return syscallWrapper(SYS_UNLINKAT, atfd, path, uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }

inline result_t sys_readdir(fd_t fd, char* buf, size_t length) { return syscallWrapper(SYS_READDIR, fd, buf, length, uword_t(0), uword_t(0), uword_t(0)); }

inline ssize_t sys_seek(fd_t fd, Anchor anchor, off_t offset)     { return syscallWrapper(SYS_SEEK,   fd, anchor, offset, uword_t(0), uword_t(0), uword_t(0)); }
inline ssize_t sys_read(fd_t fd, char* buf, size_t length)        { return syscallWrapper(SYS_READ,   fd, buf,     length,     uword_t(0), uword_t(0), uword_t(0)); }
inline ssize_t sys_write(fd_t fd, const char* buf, size_t length) { return syscallWrapper(SYS_WRITE,  fd, buf,     length,     uword_t(0), uword_t(0), uword_t(0)); }
inline result_t sys_resize(fd_t fd, size_t size)                   { return syscallWrapper(SYS_RESIZE, fd, size,    uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }

inline result_t sys_close(fd_t fd) { return syscallWrapper(SYS_CLOSE,    fd,         uword_t(0), uword_t(0),    uword_t(0), uword_t(0), uword_t(0)); }

inline result_t sys_test()                              { return syscallWrapper(SYS_TEST,     uword_t(0), uword_t(0), uword_t(0),    uword_t(0), uword_t(0), uword_t(0)); }
inline result_t sys_log(const char* msg, size_t length) { return syscallWrapper(SYS_LOG,      msg,        length,     uword_t(0),    uword_t(0), uword_t(0), uword_t(0)); }

