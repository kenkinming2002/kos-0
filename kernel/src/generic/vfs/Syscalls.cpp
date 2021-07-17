#include <generic/vfs/Syscalls.hpp>

#include <generic/vfs/VFS.hpp>

#include <i686/syscalls/Syscalls.hpp>
#include <i686/syscalls/Access.hpp>
#include <i686/tasks/Task.hpp>

#include <librt/StringRef.hpp>

namespace core::vfs
{
  namespace
  {
    Result<fd_t> sys_root()
    {
      auto rootFile = root();
      return tasks::Task::current->fileDescriptors.addFile(rootFile);
    }
    WRAP_SYSCALL0(_sys_root, sys_root)

    Result<result_t> sys_mountAt(fd_t atfd, const char* _mountpoint, const char* _mountableName, const char* _arg)
    {
      auto at = tasks::Task::current->fileDescriptors.getFile(atfd);
      UNWRAP(at);

      auto mountpoint = syscalls::stringFromUser(_mountpoint);
      UNWRAP(mountpoint);

      auto mountableName = syscalls::stringFromUser(_mountableName);
      UNWRAP(mountableName);

      auto arg = syscalls::stringFromUser(_arg);
      UNWRAP(arg);

      auto result = mountAt(rt::move(*at), *mountpoint, *mountableName, *arg);
      UNWRAP(result);
      return 0;
    }
    WRAP_SYSCALL4(_sys_mountAt, sys_mountAt)

    Result<result_t> sys_umountAt(fd_t atfd, const char* _mountpoint)
    {
      auto at = tasks::Task::current->fileDescriptors.getFile(atfd);
      UNWRAP(at);

      auto mountpoint = syscalls::stringFromUser(_mountpoint);
      UNWRAP(mountpoint);

      auto result = umountAt(rt::move(*at), *mountpoint);
      UNWRAP(result);
      return 0;
    }
    WRAP_SYSCALL2(_sys_umountAt, sys_umountAt)

    Result<fd_t> sys_openAt(fd_t atfd, const char* _path)
    {
      auto at = tasks::Task::current->fileDescriptors.getFile(atfd);
      UNWRAP(at);

      auto path = syscalls::stringFromUser(_path);
      UNWRAP(path);

      auto file = openAt(rt::move(*at), *path);
      UNWRAP(file);
      return tasks::Task::current->fileDescriptors.addFile(rt::move(*file));
    }
    WRAP_SYSCALL2(_sys_openAt, sys_openAt)

    Result<fd_t> sys_createAt(fd_t atfd, const char* _path, uword_t _type)
    {
      auto at =   tasks::Task::current->fileDescriptors.getFile(atfd);
      UNWRAP(at);

      auto path = syscalls::stringFromUser(_path);
      UNWRAP(path);

      auto type = syscalls::typeFromUser(_type);
      UNWRAP(type);

      auto file = createAt(rt::move(*at), *path, *type);
      UNWRAP(file);

      return tasks::Task::current->fileDescriptors.addFile(rt::move(*file));
    }
    WRAP_SYSCALL3(_sys_createAt, sys_createAt)

    Result<result_t> sys_linkAt(fd_t atfd, const char* _path, const char* _target)
    {
      auto at =     tasks::Task::current->fileDescriptors.getFile(atfd);
      UNWRAP(at);

      auto path =   syscalls::stringFromUser(_path);
      UNWRAP(path);

      auto target = syscalls::stringFromUser(_target);
      UNWRAP(target);

      auto result = linkAt(rt::move(*at), *path, *target);
      UNWRAP(result);
      return 0;
    }
    WRAP_SYSCALL3(_sys_linkAt, sys_linkAt)

    Result<result_t> sys_unlinkAt(fd_t atfd, const char* _path)
    {
      auto at =     tasks::Task::current->fileDescriptors.getFile(atfd);
      UNWRAP(at);

      auto path =   syscalls::stringFromUser(_path);
      UNWRAP(path);

      auto result = unlinkAt(rt::move(*at), *path);
      UNWRAP(result);
      return 0;
    }
    WRAP_SYSCALL2(_sys_unlinkAt, sys_unlinkAt)

    struct DirectoryEntry
    {
      uword_t length; // This is needed to maintain forward compatiblility - length of the entire structure
      uword_t ino;
      uword_t type;

      char name[];
    };

    Result<ssize_t> sys_readdir(fd_t fd, char* buf, size_t length)
    {
      auto file = tasks::Task::current->fileDescriptors.getFile(fd);
      UNWRAP(file);

      return (*file)->readdir(buf, length);
    }
    WRAP_SYSCALL3(_sys_readdir, sys_readdir)

    Result<ssize_t> sys_seek(fd_t fd, uword_t _anchor, off_t offset)
    {
      auto file = tasks::Task::current->fileDescriptors.getFile(fd);
      UNWRAP(file);

      auto anchor = syscalls::anchorFromUser(_anchor);
      UNWRAP(anchor);

      return (*file)->seek(*anchor, offset);
    }
    WRAP_SYSCALL3(_sys_seek, sys_seek)

    Result<ssize_t> sys_read(fd_t fd, char* buf, size_t length)
    {
      auto file = tasks::Task::current->fileDescriptors.getFile(fd);
      UNWRAP(file);

      return (*file)->read(buf, length);
    }
    WRAP_SYSCALL3(_sys_read, sys_read)

    Result<ssize_t> sys_write(fd_t fd, const char* buf, size_t length)
    {
      auto file = tasks::Task::current->fileDescriptors.getFile(fd);
      UNWRAP(file);

      return (*file)->write(buf, length);
    }
    WRAP_SYSCALL3(_sys_write, sys_write)

    Result<result_t> sys_resize(fd_t fd, size_t size)
    {
      auto file = tasks::Task::current->fileDescriptors.getFile(fd);
      UNWRAP(file);

      auto result = (*file)->resize(size);
      UNWRAP(result);
      return 0;
    }
    WRAP_SYSCALL2(_sys_resize, sys_resize)

    Result<result_t> sys_close(fd_t fd)
    {
      auto result = tasks::Task::current->fileDescriptors.removeFile(fd);
      UNWRAP(result);
      return 0;
    }
    WRAP_SYSCALL1(_sys_close, sys_close)
  }

  void initializeSyscalls()
  {
    syscalls::installHandler(SYS_ROOT    , &_sys_root    );
    syscalls::installHandler(SYS_MOUNTAT , &_sys_mountAt );
    syscalls::installHandler(SYS_UMOUNTAT, &_sys_umountAt);
    syscalls::installHandler(SYS_OPENAT  , &_sys_openAt  );
    syscalls::installHandler(SYS_CREATEAT, &_sys_createAt);
    syscalls::installHandler(SYS_LINKAT  , &_sys_linkAt  );
    syscalls::installHandler(SYS_UNLINKAT, &_sys_unlinkAt);
    syscalls::installHandler(SYS_READDIR , &_sys_readdir );
    syscalls::installHandler(SYS_SEEK    , &_sys_seek    );
    syscalls::installHandler(SYS_READ    , &_sys_read    );
    syscalls::installHandler(SYS_WRITE   , &_sys_write   );
    syscalls::installHandler(SYS_RESIZE  , &_sys_resize  );
    syscalls::installHandler(SYS_CLOSE   , &_sys_close   );
  }
}
