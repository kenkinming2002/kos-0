#pragma once

#include <sys/Types.hpp>
#include <sys/Syscalls.hpp>

#include <librt/Cast.hpp>
#include <librt/Utility.hpp>

extern "C" word_t syscall(uword_t syscallNumber, ...);

inline result_t sys_root() { return syscall(SYS_ROOT, uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }

inline result_t sys_mountAt(fd_t atfd, const char* mountpoint, const char* mountableName, const char* arg) { return syscall(SYS_MOUNTAT, atfd, mountpoint, mountableName, arg, uword_t(0), uword_t(0)); }
inline result_t sys_umountAt(fd_t atfd, const char* mountpoint) { return syscall(SYS_UMOUNTAT, atfd, mountpoint, uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }

inline fd_t sys_openAt(fd_t atfd, const char* path) { return syscall(SYS_OPENAT, atfd, path, uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }
inline fd_t sys_createAt(fd_t atfd, const char* path, Type type) { return syscall(SYS_CREATEAT, atfd, path, type, uword_t(0), uword_t(0), uword_t(0)); }
inline result_t sys_linkAt(fd_t atfd, const char* path, const char* target) { return syscall(SYS_LINKAT, atfd, path, target, uword_t(0), uword_t(0), uword_t(0)); }
inline result_t sys_unlinkAt(fd_t atfd, const char* path) { return syscall(SYS_UNLINKAT, atfd, path, uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }

inline result_t sys_readdir(fd_t fd, char* buf, size_t length) { return syscall(SYS_READDIR, fd, buf, length, uword_t(0), uword_t(0), uword_t(0)); }

inline ssize_t sys_seek(fd_t fd, Anchor anchor, off_t offset) { return syscall(SYS_SEEK, fd, anchor, offset, uword_t(0), uword_t(0), uword_t(0)); }
inline ssize_t sys_read(fd_t fd, char* buf, size_t length) { return syscall(SYS_READ, fd, buf, length, uword_t(0), uword_t(0), uword_t(0)); }
inline ssize_t sys_write(fd_t fd, const char* buf, size_t length) { return syscall(SYS_WRITE, fd, buf, length, uword_t(0), uword_t(0), uword_t(0)); }
inline result_t sys_resize(fd_t fd, size_t size) { return syscall(SYS_RESIZE, fd, size, uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }

inline result_t sys_close(fd_t fd) { return syscall(SYS_CLOSE, fd, uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }

inline result_t sys_mmap(uintptr_t addr, size_t length, Prot prot, fd_t fd, size_t offset) { return syscall(SYS_MMAP, addr, length, prot, fd, offset, uword_t(0)); }
inline result_t sys_munmap(uintptr_t addr, size_t length) { return syscall(SYS_MUNMAP, addr, length, uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }
inline result_t sys_mremap(uintptr_t addr, size_t length, size_t newLength) { return syscall(SYS_MREMAP, addr, length, newLength, uword_t(0), uword_t(0), uword_t(0)); }

inline pid_t sys_getpid() { return syscall(SYS_GETPID, uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }
inline result_t sys_yield() { return syscall(SYS_YIELD, uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }
inline result_t sys_kill(pid_t pid, status_t status) { return syscall(SYS_KILL, pid, status, uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }
inline pid_t sys_fork() { return syscall(SYS_FORK, uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }

inline result_t sys_test() { return syscall(SYS_TEST, uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }
inline result_t sys_log(const char* msg, size_t length) { return syscall(SYS_LOG, msg, length, uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }

[[noreturn]] inline void _exit(status_t status)
{
  sys_kill(sys_getpid(), status);
  __builtin_unreachable();
}
