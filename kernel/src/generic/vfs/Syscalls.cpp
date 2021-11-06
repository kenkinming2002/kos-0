#include <generic/vfs/Syscalls.hpp>

#include <generic/vfs/VFS.hpp>
#include <generic/tasks/Scheduler.hpp>

#include <i686/syscalls/Syscalls.hpp>
#include <i686/syscalls/Access.hpp>

#include <librt/Log.hpp>

namespace core::vfs
{
  namespace
  {
    template<typename T> inline result_t makeError(const Result<T>& result) { return -static_cast<result_t>(result.error()); }
    template<typename T> inline result_t convertResult(const Result<T>& result) { return result ? *result : makeError(result); }
    template<>           inline result_t convertResult(const Result<void>& result) { return result ? 0 : makeError(result); }

    auto success()
    {
      return result_t(0);
    }

    auto addFile(rt::SharedPtr<File> file)
    {
      return tasks::current().fileDescriptors->addFile(rt::move(file));
    }

    Result<result_t> dispatch(const VfsCommand& command)
    {
      auto at = command.fd != ROOT_FD ? tasks::current().fileDescriptors->getFile(command.fd) : root();
      if(!at)
        return makeError(at);

      switch(command.opcode)
      {
        case VfsCommand::Opcode::MOUNT:
        {
          auto mountpoint = syscalls::stringFromUser(command.mount.mountpoint);
          if(!mountpoint)
            return mountpoint.error();

          auto mountableName = syscalls::stringFromUser(command.mount.mountableName);
          if(!mountableName)
            return mountableName.error();

          auto arg = syscalls::stringFromUser(command.mount.arg);
          if(!arg)
            return arg.error();

          return mountAt(*at, *mountpoint, *mountableName, *arg).map(&success);
        }
        case VfsCommand::Opcode::UMOUNT:
        {
          auto mountpoint = syscalls::stringFromUser(command.umount.mountpoint);
          if(!mountpoint)
            return mountpoint.error();

          return umountAt(*at, *mountpoint).map(&success);
        }
        case VfsCommand::Opcode::OPEN:
        {
          auto path = syscalls::stringFromUser(command.open.path);
          if(!path)
            return path.error();

          return openAt(*at, *path).andThen(&addFile);
        }
        case VfsCommand::Opcode::CREATE:
        {
          auto path = syscalls::stringFromUser(command.create.path);
          if(!path)
            return path.error();

          auto type = command.create.type;
          switch(static_cast<Type>(type))
          {
          case Type::REGULAR_FILE:
          case Type::DIRECTORY:
          case Type::SYMBOLIC_LINK:
          case Type::OTHER:
            break;
          default:
            return ErrorCode::INVALID;
          }
          return createAt(*at, *path, type).andThen(&addFile);
        }
        case VfsCommand::Opcode::LINK:
        {
          auto path = syscalls::stringFromUser(command.link.path);
          if(!path)
            return path.error();

          auto target = syscalls::stringFromUser(command.link.target);
          if(!target)
            return target.error();

          return linkAt(*at, *path, *target).map(&success);
        }
        case VfsCommand::Opcode::UNLINK:
        {
          auto path = syscalls::stringFromUser(command.unlink.path);
          if(!path)
            return path.error();

          return unlinkAt(*at, *path).map(&success);
        }
        case VfsCommand::Opcode::READDIR:
          return (*at)->readdir(command.readdir.buf, command.readdir.length);
        case VfsCommand::Opcode::SEEK:
        {
          auto anchor = command.seek.anchor;
          switch(static_cast<Anchor>(anchor))
          {
          case Anchor::BEGIN:
          case Anchor::CURRENT:
          case Anchor::END:
            break;
          default:
            return ErrorCode::INVALID;
          }
          return (*at)->seek(anchor, command.seek.offset);
        }
        case VfsCommand::Opcode::READ:
          return (*at)->read(command.read.buf, command.read.length);
        case VfsCommand::Opcode::WRITE:
          return (*at)->write(command.write.buf, command.write.length);
        case VfsCommand::Opcode::RESIZE:
          return (*at)->resize(command.resize.size).map(&success);
        case VfsCommand::Opcode::CLOSE:
          return tasks::current().fileDescriptors->removeFile(command.fd).map(&success);
        default:
          return ErrorCode::INVALID;
      }
    }

    Result<vfs_command_t> sys_async_submit(VfsCommand* command)
    {
      // TODO: Verify command ptr is valid
      return tasks::current().commandQueue.submit(*command);
    }
    WRAP_SYSCALL1(_sys_async_submit, sys_async_submit)

    Result<vfs_command_t> sys_async_wait(result_t* _result)
    {
      VfsCommand command;
      vfs_command_t handle;

      // TODO: Decide smartly which command to dispatch
      handle = tasks::current().commandQueue.retrive(command);
      if(handle == -1)
        return ErrorCode::INVALID; // We are ask to dispatch an io operation but none are submitted

      auto result = dispatch(command);
      *_result = result ? *result : -static_cast<result_t>(result.error());
      return handle;
    }
    WRAP_SYSCALL1(_sys_async_wait, sys_async_wait)
  }

  void initializeSyscalls()
  {
    syscalls::installHandler(SYS_ASYNC_SUBMIT, &_sys_async_submit);
    syscalls::installHandler(SYS_ASYNC_WAIT  , &_sys_async_wait);
  }
}
