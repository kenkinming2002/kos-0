#include <generic/vfs/Mountable.hpp>

#include <generic/vfs/mountables/devices/Serial.hpp>
#include <generic/vfs/mountables/filesystems/Tmpfs.hpp>

#include <librt/Algorithm.hpp>
#include <librt/Global.hpp>
#include <generic/SpinLock.hpp>

namespace core::vfs
{
  namespace
  {
    // TODO: Maybe someform of reference wrappers or intrusive list?
    constinit core::SpinLock lock;
    constinit rt::Global<rt::containers::IntrusiveList<Mountable>> mountables;
  }

  void initializeMountables()
  {
    mountables.construct();
    initializeTmpfs();
    initializeSerial();
  }

  void registerMountable(Mountable& mountable)
  {
    core::LockGuard guard(lock);
    mountables().insert(mountables().end(), mountable);
  }

  void deregisterMountable(Mountable& mountable)
  {
    core::LockGuard guard(lock);
    auto it = rt::find_if(mountables().begin(), mountables().end(), [&mountable](Mountable& _mountable){
        return mountable.name() == _mountable.name();
    });

    ASSERT(it != mountables().end());
    mountables().remove_and_dispose(it, [](auto ptr) {});
  }

  Mountable* lookupMountable(rt::StringRef name)
  {
    core::LockGuard guard(lock);
    auto it = rt::find_if(mountables().begin(), mountables().end(), [&name](Mountable& mountable){
        return mountable.name() == name;
    });
    return it != mountables().end() ? &(*it) : nullptr;
  }
}
