#include <generic/vfs/VFS.hpp>

#include <generic/vfs/Syscalls.hpp>
#include <generic/vfs/Mountable.hpp>
#include <generic/vfs/Path.hpp>
#include <generic/vfs/Initrd.hpp>

#include <librt/Global.hpp>

namespace core::vfs
{
  namespace
  {
    constinit rt::Global<rt::SharedPtr<Vnode>> rootVnode;

    void initializeRoot()
    {
      rootVnode.construct(rt::makeShared<Vnode>(nullptr, rt::makeShared<Inode>()));

      auto* tmpfs = lookupMountable("tmpfs");
      auto rootFile = root();
      mountAt(rootFile, "", *tmpfs, {});
    }
  }

  void initialize()
  {
    initializeMountables();
    initializeSyscalls();
    initializeRoot();
    loadInitrd();
  }

  rt::SharedPtr<File> root() { return rt::makeShared<File>(rootVnode()); }

  Result<void> mountAt(rt::SharedPtr<File> at, rt::StringRef mountpoint, rt::StringRef mountableName, rt::StringRef arg)
  {
    auto* mountable = lookupMountable(mountableName);
    if(!mountable)
      return ErrorCode::INVALID;

    return mountAt(at, mountpoint, *mountable, arg);
  }

  Result<void> mountAt(rt::SharedPtr<File> at, rt::StringRef mountpoint, Mountable& mountable, rt::StringRef arg)
  {
    auto file = openAt(at, mountpoint);
    if(!file)
      return file.error();

    return (*file)->mount(mountable, arg);
  }

  Result<void> umountAt(rt::SharedPtr<File> at, rt::StringRef mountpoint)
  {
    auto file = openAt(at, mountpoint);
    if(!file)
      return file.error();

    return (*file)->umount();
  }

  Result<rt::SharedPtr<File>> openAt(rt::SharedPtr<File> at, rt::StringRef path)
  {
    path = path::trimLeadingSlash(path);

    auto file = at;
    for(auto component : path::components(path))
    {
      auto result = (*file).lookup(component);
      if(!result)
        return result.error();

      file = *result;
    }
    return file;
  }

  Result<rt::SharedPtr<File>> createAt(rt::SharedPtr<File> at, rt::StringRef path, Type type)
  {
    auto [dirname, basename] = path::splitLast(path);
    auto file = openAt(at, dirname);
    if(!file)
      return file.error();

    return (*file)->create(basename, type);
  }

  Result<void> linkAt(rt::SharedPtr<File> at, rt::StringRef path, rt::StringRef target)
  {
    // We need shared ptr to support this properly
    return ErrorCode::UNSUPPORTED;
  }

  Result<void> unlinkAt(rt::SharedPtr<File> at, rt::StringRef path)
  {
    auto [dirname, basename] = path::splitLast(path);
    auto file = openAt(at, dirname);
    if(!file)
      return file.error();

    return (*file)->unlink(basename);
  }
}
