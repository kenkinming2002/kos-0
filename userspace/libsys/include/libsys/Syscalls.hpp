#pragma once

#include <sys/Types.hpp>
#include <sys/Syscalls.hpp>
#include <sys/VfsCommand.hpp>

#include <librt/Cast.hpp>
#include <librt/Utility.hpp>

extern "C" word_t syscall(uword_t syscallNumber, ...);

// VFS
inline vfs_command_t sys_async_submit(VfsCommand* command) { return syscall(SYS_ASYNC_SUBMIT, command, uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }
inline vfs_command_t sys_async_wait(result_t* result)      { return syscall(SYS_ASYNC_WAIT, result,  uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }

inline result_t sys_mountAt(fd_t atfd, const char* mountpoint, const char* mountableName, const char* arg)
{
  result_t result = -1;
  VfsCommand command = {};

  command.opcode = VfsCommand::Opcode::MOUNT;
  command.fd = atfd;
  command.mount.mountpoint = mountpoint;
  command.mount.mountableName = mountableName;
  command.mount.arg = arg;
  sys_async_submit(&command);
  sys_async_wait(&result);
  return result;
}

inline result_t sys_umountAt(fd_t atfd, const char* mountpoint)
{
  result_t result = -1;
  VfsCommand command = {};

  command.opcode = VfsCommand::Opcode::UMOUNT;
  command.fd = atfd;
  command.umount.mountpoint = mountpoint;
  sys_async_submit(&command);
  sys_async_wait(&result);
  return result;
}

inline fd_t sys_openAt(fd_t atfd, const char* path)
{
  result_t result = -1;
  VfsCommand command = {};

  command.opcode = VfsCommand::Opcode::OPEN;
  command.fd = atfd;
  command.open.path = path;
  sys_async_submit(&command);
  sys_async_wait(&result);
  return result;
}

inline fd_t sys_createAt(fd_t atfd, const char* path, Type type)
{
  result_t result = -1;
  VfsCommand command = {};

  command.opcode = VfsCommand::Opcode::CREATE;
  command.fd = atfd;
  command.create.path = path;
  command.create.type = type;
  sys_async_submit(&command);
  sys_async_wait(&result);
  return result;
}

inline result_t sys_linkAt(fd_t atfd, const char* path, const char* target)
{
  result_t result = -1;
  VfsCommand command = {};

  command.opcode = VfsCommand::Opcode::LINK;
  command.fd = atfd;
  command.link.path   = path;
  command.link.target = target;
  sys_async_submit(&command);
  sys_async_wait(&result);
  return result;
}

inline result_t sys_unlinkAt(fd_t atfd, const char* path)
{
  result_t result = -1;
  VfsCommand command = {};

  command.opcode = VfsCommand::Opcode::UNLINK;
  command.fd = atfd;
  command.unlink.path = path;
  sys_async_submit(&command);
  sys_async_wait(&result);
  return result;
}


inline result_t sys_readdir(fd_t fd, char* buf, size_t length)
{
  result_t result = -1;
  VfsCommand command = {};

  command.opcode = VfsCommand::Opcode::READDIR;
  command.fd = fd;
  command.readdir.buf = buf;
  command.readdir.length = length;
  sys_async_submit(&command);
  sys_async_wait(&result);
  return result;
}


inline ssize_t sys_seek(fd_t fd, Anchor anchor, off_t offset)
{
  result_t result = -1;
  VfsCommand command = {};

  command.opcode = VfsCommand::Opcode::SEEK;
  command.fd = fd;
  command.seek.anchor = anchor;
  command.seek.offset = offset;
  sys_async_submit(&command);
  sys_async_wait(&result);
  return result;
}

inline ssize_t sys_read(fd_t fd, char* buf, size_t length)
{
  result_t result = -1;
  VfsCommand command = {};

  command.opcode = VfsCommand::Opcode::READ;
  command.fd = fd;
  command.read.buf = buf;
  command.read.length = length;
  sys_async_submit(&command);
  sys_async_wait(&result);
  return result;
}

inline ssize_t sys_write(fd_t fd, const char* buf, size_t length)
{
  result_t result = -1;
  VfsCommand command = {};

  command.opcode = VfsCommand::Opcode::WRITE;
  command.fd = fd;
  command.write.buf = buf;
  command.write.length = length;
  sys_async_submit(&command);
  sys_async_wait(&result);
  return result;
}

inline result_t sys_resize(fd_t fd, size_t size)
{
  result_t result = -1;
  VfsCommand command = {};

  command.opcode = VfsCommand::Opcode::RESIZE;
  command.fd = fd;
  command.resize.size = size;
  sys_async_submit(&command);
  sys_async_wait(&result);
  return result;
}


inline result_t sys_close(fd_t fd)
{
  result_t result = -1;
  VfsCommand command = {};

  command.opcode = VfsCommand::Opcode::CLOSE;
  command.fd = fd;
  sys_async_submit(&command);
  sys_async_wait(&result);
  return result;
}


// Memory
inline result_t sys_mmap(uintptr_t addr, size_t length, Prot prot, fd_t fd, size_t fileOffset, size_t fileLength) { return syscall(SYS_MMAP, addr, length, prot, fd, fileOffset, fileLength); }
inline result_t sys_munmap(uintptr_t addr, size_t length) { return syscall(SYS_MUNMAP, addr, length, uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }
inline result_t sys_mremap(uintptr_t addr, size_t length, size_t newLength) { return syscall(SYS_MREMAP, addr, length, newLength, uword_t(0), uword_t(0), uword_t(0)); }

// Task
inline pid_t sys_getpid() { return syscall(SYS_GETPID, uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }
inline result_t sys_yield() { return syscall(SYS_YIELD, uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }
inline result_t sys_kill(pid_t pid, status_t status) { return syscall(SYS_KILL, pid, status, uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }
inline pid_t sys_fork() { return syscall(SYS_FORK, uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }

// Misc
inline result_t sys_test() { return syscall(SYS_TEST, uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }
inline result_t sys_log(const char* msg, size_t length) { return syscall(SYS_LOG, msg, length, uword_t(0), uword_t(0), uword_t(0), uword_t(0)); }

[[noreturn]] inline void _exit(status_t status)
{
  sys_kill(sys_getpid(), status);
  __builtin_unreachable();
}
