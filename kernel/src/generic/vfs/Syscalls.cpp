#include <generic/vfs/Syscalls.hpp>

#include <generic/vfs/VFS.hpp>

#include <i686/syscalls/Syscalls.hpp>
#include <i686/syscalls/Access.hpp>
#include <i686/tasks/Task.hpp>

#include <librt/StringRef.hpp>

namespace core::vfs
{
#define UNWRAP(v, expr) auto v = expr; if(!(v)) return -static_cast<result_t>(v.error())

  static result_t _sys_root()
  {
    auto rootFile = root();
    UNWRAP(result, tasks::Task::current()->fileDescriptors().addFile(rootFile));
    return *result;
  }
  WRAP_SYSCALL0(sys_root, _sys_root)

  static result_t _sys_mountAt(fd_t atfd, const char* _mountpoint, const char* _mountableName, const char* _arg)
  {
    UNWRAP(at,            tasks::Task::current()->fileDescriptors().getFile(atfd));
    UNWRAP(mountpoint,    syscalls::stringFromUser(_mountpoint));
    UNWRAP(mountableName, syscalls::stringFromUser(_mountableName));
    UNWRAP(arg,           syscalls::stringFromUser(_arg));

    UNWRAP(_, mountAt(*at, *mountpoint, *mountableName, *arg));
    return 0;
  }
  WRAP_SYSCALL4(sys_mountAt, _sys_mountAt)

  static result_t _sys_umountAt(fd_t atfd, const char* _mountpoint)
  {
    UNWRAP(at,         tasks::Task::current()->fileDescriptors().getFile(atfd));
    UNWRAP(mountpoint, syscalls::stringFromUser(_mountpoint));

    UNWRAP(_, umountAt(*at, *mountpoint));
    return 0;
  }
  WRAP_SYSCALL2(sys_umountAt, _sys_umountAt)

  static fd_t _sys_openAt(fd_t atfd, const char* _path)
  {
    UNWRAP(at,   tasks::Task::current()->fileDescriptors().getFile(atfd));
    UNWRAP(path, syscalls::stringFromUser(_path));

    UNWRAP(file, openAt(*at, *path));
    UNWRAP(result, tasks::Task::current()->fileDescriptors().addFile(*file));

    return *result;
  }
  WRAP_SYSCALL2(sys_openAt, _sys_openAt)

  static fd_t _sys_createAt(fd_t atfd, const char* _path, uword_t _type)
  {
    UNWRAP(at,   tasks::Task::current()->fileDescriptors().getFile(atfd));
    UNWRAP(path, syscalls::stringFromUser(_path));
    UNWRAP(type, typeFromUser(_type));

    UNWRAP(file, createAt(*at, *path, *type));
    UNWRAP(result, tasks::Task::current()->fileDescriptors().addFile(*file));
    return *result;
  }
  WRAP_SYSCALL3(sys_createAt, _sys_createAt)

  static result_t _sys_linkAt(fd_t atfd, const char* _path, const char* _target)
  {
    UNWRAP(at,     tasks::Task::current()->fileDescriptors().getFile(atfd));
    UNWRAP(path,   syscalls::stringFromUser(_path));
    UNWRAP(target, syscalls::stringFromUser(_target));

    UNWRAP(_, linkAt(*at, *path, *target));
    return 0;
  }
  WRAP_SYSCALL3(sys_linkAt, _sys_linkAt)

  static result_t _sys_unlinkAt(fd_t atfd, const char* _path)
  {
    UNWRAP(at,     tasks::Task::current()->fileDescriptors().getFile(atfd));
    UNWRAP(path,   syscalls::stringFromUser(_path));

    UNWRAP(_, unlinkAt(*at, *path));
    return 0;
  }
  WRAP_SYSCALL2(sys_unlinkAt, _sys_unlinkAt)

  struct DirectoryEntry
  {
    uword_t length; // This is needed to maintain forward compatiblility - length of the entire structure
    uword_t ino;
    uword_t type;

    char name[];
  };

  static result_t _sys_readdir(fd_t fd, char* buf, size_t length)
  {
    UNWRAP(file, tasks::Task::current()->fileDescriptors().getFile(fd));
    UNWRAP(result, file->readdir(buf, length));

    return *result;
  }
  WRAP_SYSCALL3(sys_readdir, _sys_readdir)

  static ssize_t _sys_seek(fd_t fd, uword_t _anchor, off_t offset)
  {
    UNWRAP(file, tasks::Task::current()->fileDescriptors().getFile(fd));
    UNWRAP(anchor, anchorFromUser(_anchor));
    UNWRAP(result, file->seek(*anchor, offset));
    return *result;
  }
  WRAP_SYSCALL3(sys_seek, _sys_seek)

  static ssize_t _sys_read(fd_t fd, char* buf, size_t length)
  {
    UNWRAP(file, tasks::Task::current()->fileDescriptors().getFile(fd));
    UNWRAP(result, file->read(buf, length));
    return *result;
  }
  WRAP_SYSCALL3(sys_read, _sys_read)

  static ssize_t _sys_write(fd_t fd, const char* buf, size_t length)
  {
    UNWRAP(file, tasks::Task::current()->fileDescriptors().getFile(fd));
    UNWRAP(result, file->write(buf, length));
    return *result;
  }
  WRAP_SYSCALL3(sys_write, _sys_write)

  static result_t _sys_resize(fd_t fd, size_t size)
  {
    UNWRAP(file, tasks::Task::current()->fileDescriptors().getFile(fd));
    UNWRAP(_, file->resize(size));
    return 0;
  }
  WRAP_SYSCALL2(sys_resize, _sys_resize)

  static result_t _sys_close(fd_t fd)
  {
    UNWRAP(_, tasks::Task::current()->fileDescriptors().removeFile(fd));
    return 0;
  }
  WRAP_SYSCALL1(sys_close, _sys_close)

#undef UNWRAP

  void initializeSyscalls()
  {
    syscalls::installHandler(syscalls::SYS_ROOT    , &sys_root    );
    syscalls::installHandler(syscalls::SYS_MOUNTAT , &sys_mountAt );
    syscalls::installHandler(syscalls::SYS_UMOUNTAT, &sys_umountAt);
    syscalls::installHandler(syscalls::SYS_OPENAT  , &sys_openAt  );
    syscalls::installHandler(syscalls::SYS_CREATEAT, &sys_createAt);
    syscalls::installHandler(syscalls::SYS_LINKAT  , &sys_linkAt  );
    syscalls::installHandler(syscalls::SYS_UNLINKAT, &sys_unlinkAt);
    syscalls::installHandler(syscalls::SYS_READDIR , &sys_readdir );
    syscalls::installHandler(syscalls::SYS_SEEK    , &sys_seek    );
    syscalls::installHandler(syscalls::SYS_READ    , &sys_read    );
    syscalls::installHandler(syscalls::SYS_WRITE   , &sys_write   );
    syscalls::installHandler(syscalls::SYS_RESIZE  , &sys_resize  );
    syscalls::installHandler(syscalls::SYS_CLOSE   , &sys_close   );
  }
}
