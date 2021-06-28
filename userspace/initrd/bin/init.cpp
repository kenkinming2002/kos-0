#include <libsys/Syscalls.hpp>
#include <libsys/brk.hpp>

#include <librt/Strings.hpp>
#include <librt/Log.hpp>
#include <librt/Assert.hpp>

#include <stddef.h>

extern "C" char _end[];

struct Dirent
{
  uword_t length; // This is needed to maintain forward compatiblility - length of the entire structure
  uword_t ino;
  uword_t type;

  char name[];
};

void lsdir(fd_t fd, size_t depth = 0)
{
  if(depth == 10)
    return;

  char buf[128];
  ssize_t count = sys_readdir(fd, buf, sizeof buf);
  if(count < 0)
    return; // Perhaps fd does not refer to a directory afterall? Do not make this an error.

  size_t offset = 0;
  while(offset < size_t(count))
  {
    for(size_t i=0; i<depth; ++i)
      rt::log("  ");

    auto* entry = reinterpret_cast<const Dirent*>(buf+offset);
    rt::logf("%s\n", entry->name);

    auto child_fd = sys_openAt(fd, entry->name);
    if(child_fd < 0)
    {
      rt::log("openAt\n");
      continue;
    }
    lsdir(child_fd, depth+1);
    sys_close(child_fd);

    offset += entry->length;
  }
}

void test()
{
  auto rootfd = sys_root();
  ASSERT_ALWAYS(rootfd >= 0);

  ASSERT_ALWAYS(sys_openAt(rootfd, "test") < 0);
  ASSERT_ALWAYS(sys_openAt(rootfd, "test4") < 0);
  ASSERT_ALWAYS(sys_openAt(rootfd, "test3") < 0);

  ASSERT_ALWAYS(sys_createAt(rootfd, "test",  Type::REGULAR_FILE) >= 0);
  ASSERT_ALWAYS(sys_createAt(rootfd, "test4", Type::REGULAR_FILE) >= 0);
  ASSERT_ALWAYS(sys_createAt(rootfd, "test3", Type::REGULAR_FILE) >= 0);

  {
    auto testfd = sys_openAt(rootfd, "test");
    ASSERT_ALWAYS(testfd >= 0);

    constexpr auto OFFSET = 0xFF8;
    const char writeBuf[] = "HelloWorld";
    char readBuf[5]       = {};

    ASSERT_ALWAYS(sys_resize(testfd, OFFSET+sizeof writeBuf) == 0);

    ASSERT_ALWAYS(sys_seek(testfd, Anchor::BEGIN, OFFSET) == OFFSET);
    ASSERT_ALWAYS(sys_write(testfd, writeBuf, sizeof writeBuf) == sizeof writeBuf);

    ASSERT_ALWAYS(sys_seek(testfd, Anchor::BEGIN, OFFSET+2) == OFFSET+2);
    ASSERT_ALWAYS(sys_read(testfd, readBuf, sizeof readBuf) == sizeof readBuf);

    rt::log("Result:");
    rt::log(readBuf, 5);
    rt::log("\n");
  }

  {
    char* mem = static_cast<char*>(sbrk(0x1000));
    ASSERT_ALWAYS(mem != reinterpret_cast<char*>(-1));
    for(size_t i=0; i<0x1000; ++i)
      mem[i] = 'K';
  }

  {
    for(size_t i=0; i<10; ++i)
    {
      char* mem = static_cast<char*>(sbrk(0x1000));
      ASSERT_ALWAYS(mem != reinterpret_cast<char*>(-1));
      for(size_t i=0; i<0x1000; ++i)
        mem[i] = 'A';
      ASSERT_ALWAYS(sbrk(-0x1000) != reinterpret_cast<void*>(-1));
    }
  }

  {
    ASSERT_ALWAYS(sbrk(-0x1000) != reinterpret_cast<void*>(-1));
  }

  {
    auto serial1fd = sys_createAt(rootfd, "serial1", Type::REGULAR_FILE);
    ASSERT_ALWAYS(serial1fd >= 0);
    ASSERT_ALWAYS(sys_mountAt(serial1fd, "", "serial", "1") == 0);

    for(;;)
    {
      size_t result;
      char buf[1];

      result = sys_read(serial1fd, buf, sizeof buf);

      rt::log(buf, result);
      result = sys_write(serial1fd, buf, sizeof buf);
      if(result<0)
        rt::log("Error occured\n");
    }
  }
}

extern "C" void main()
{
  auto rootfd = sys_root();
  if(rootfd != 0)
  {
    rt::log("root()\n");
    return;
  }

  test();
  lsdir(rootfd);
}
