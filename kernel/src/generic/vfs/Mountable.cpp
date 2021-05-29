#include <generic/vfs/Mountable.hpp>

#include <generic/vfs/mountables/filesystems/Tmpfs.hpp>

#include <librt/Algorithm.hpp>
#include <librt/Global.hpp>
#include <librt/containers/List.hpp>

namespace core::vfs
{
  namespace
  {
    // TODO: Maybe someform of reference wrappers or intrusive list?
    constinit rt::Global<rt::containers::List<Mountable*>> mountables;
    constinit rt::Global<Tmpfs> tmpfs; // Tmpfs is always available because that is the default root mountable
  }

  void initializeMountables()
  {
    mountables.construct();
    tmpfs.construct();
    registerMountable(tmpfs());
  }

  void registerMountable(Mountable& mountable)
  {
    mountables().insert(mountables().end(), &mountable);
  }

  void deregisterMountable(Mountable& mountable)
  {
    auto it = rt::find(mountables().begin(), mountables().end(), &mountable);
    ASSERT(it != mountables().end());
    mountables().erase(it);
  }

  Mountable* lookupMountable(rt::StringRef name)
  {
    auto it = rt::find_if(mountables().begin(), mountables().end(), [&name](Mountable* mountable){
        return mountable->name() == name;
    });
    return it != mountables().end() ? *it : nullptr;
  }
}
