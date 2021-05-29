#include <generic/vfs/VFS.hpp>

#include <generic/vfs/Path.hpp>
#include <generic/vfs/Initrd.hpp>

#include <generic/vfs/mountables/Null.hpp>

#include <librt/SharedPtr.hpp>
#include <librt/Result.hpp>
#include <librt/Global.hpp>
#include <librt/StringRef.hpp>
#include <librt/UniquePtr.hpp>
#include <librt/Pair.hpp>

namespace core::vfs
{
  namespace
  {
    void lsdir(core::vfs::File& file, size_t depth=0)
    {
      if(depth == 0)
        rt::log("lsdir()\n");

      auto callback = [&](Inode::DirectoryEntry directoryEntry) -> void {
        for(size_t i=0; i<depth; ++i)
          rt::log("  ");

        rt::log("Filename:");
        rt::log(directoryEntry.name.data(), directoryEntry.name.length());
        rt::log("\n");

        for(size_t i=0; i<depth; ++i)
          rt::log("  ");

        auto stat = file.stat();
        if(stat)
          rt::logf("Filesize:%lu\n", stat->size);
        else
          rt::logf("Filesize:unknown\n");

        auto child = file.lookup(directoryEntry.name);
        if(child)
          lsdir(*child, depth+1);
      };

      typedef std::remove_cvref_t<decltype(callback)> callback_t;
      file.iterate([](Inode::DirectoryEntry directoryEntry, void* data){
          (*static_cast<callback_t*>(data))(directoryEntry);
      }, &callback);
    }

    void test()
    {
      auto rootFile = root();
      lsdir(rootFile);

      ASSERT_ALWAYS(core::vfs::openAt(rootFile, "/"));

      ASSERT_ALWAYS(!core::vfs::openAt(rootFile, "/test"));
      ASSERT_ALWAYS(!core::vfs::openAt(rootFile, "/test4"));
      ASSERT_ALWAYS(!core::vfs::openAt(rootFile, "/test3"));

      ASSERT_ALWAYS(core::vfs::createAt(rootFile, "/test",  core::vfs::Type::REGULAR_FILE));
      ASSERT_ALWAYS(core::vfs::createAt(rootFile, "/test4", core::vfs::Type::REGULAR_FILE));
      ASSERT_ALWAYS(core::vfs::createAt(rootFile, "/test3", core::vfs::Type::REGULAR_FILE));

      ASSERT_ALWAYS(core::vfs::openAt(rootFile, "/test"));
      {
        auto testFile = core::vfs::openAt(rootFile, "/test");

        const char writeBuf[] = "HelloWorld";
        char readBuf[5];

        constexpr auto OFFSET = 0xFF8;
        testFile->resize(OFFSET+sizeof writeBuf);
        testFile->write(writeBuf, sizeof writeBuf, OFFSET);
        testFile->read(readBuf, sizeof readBuf, OFFSET+2);

        rt::log("Result:");
        rt::log(readBuf, 5);
        rt::log("\n");
      }

      {
        auto subdirVnode = createAt(rootFile, "subdir", core::vfs::Type::DIRECTORY);
        ASSERT(subdirVnode);
        createAt(*subdirVnode, "a", core::vfs::Type::REGULAR_FILE);
        createAt(*subdirVnode, "b", core::vfs::Type::REGULAR_FILE);
        lsdir(rootFile);
      }

      for(;;) asm("hlt");
    }
  }

  namespace
  {
    constinit rt::Global<rt::SharedPtr<Vnode>> rootVnode;

    void initializeRoot()
    {
      rootVnode.construct(rt::makeShared<Vnode>(nullptr));
      rootVnode()->associate(rt::makeShared<NullInode>());

      auto* tmpfs = lookupMountable("tmpfs");
      auto rootFile = root();
      mountAt(rootFile, "", *tmpfs, {});
    }
  }

  void initialize()
  {
    initializeMountables();
    initializeRoot();
    loadInitrd();
    test();
  }

  File root() { return File(rootVnode()); }

  Result<void> mountAt(File& at, rt::StringRef mountpoint, Mountable& mountable, rt::Span<rt::StringRef> args)
  {
    auto file = openAt(at, mountpoint);
    if(!file)
      return file.error();

    return file->mount(mountable, args);
  }

  Result<void> umountAt(File& at, rt::StringRef mountpoint)
  {
    auto file = openAt(at, mountpoint);
    if(!file)
      return file.error();

    return file->umount();
  }

  Result<File> openAt(File& at, rt::StringRef path)
  {
    path = path::trimLeadingSlash(path);

    auto file = at;
    for(auto component : path::components(path))
    {
      auto result = file.lookup(component);
      if(!result)
        return result.error();

      file = *result;
    }
    return file;
  }

  Result<File> createAt(File& at, rt::StringRef path, Type type)
  {
    auto [dirname, basename] = path::splitLast(path);
    auto file = openAt(at, dirname);
    if(!file)
      return file.error();

    return file->create(basename, type);
  }

  Result<void> linkAt(File& at, rt::StringRef path, rt::StringRef target)
  {
    // We need shared ptr to support this properly
    return ErrorCode::UNSUPPORTED;
  }

  Result<void> unlinkAt(File& at, rt::StringRef path)
  {
    auto [dirname, basename] = path::splitLast(path);
    auto file = openAt(at, dirname);
    if(!file)
      return file.error();

    return file->unlink(basename);
  }

}
