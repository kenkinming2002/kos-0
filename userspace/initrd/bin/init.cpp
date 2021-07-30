#include <libsys/Syscalls.hpp>
#include <libsys/brk.hpp>

#include <librt/Strings.hpp>
#include <librt/Log.hpp>
#include <librt/Assert.hpp>

#include <type_traits>
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

void test1()
{
  ASSERT_ALWAYS(sys_openAt(ROOT_FD, "test") < 0);
  ASSERT_ALWAYS(sys_openAt(ROOT_FD, "test4") < 0);
  ASSERT_ALWAYS(sys_openAt(ROOT_FD, "test3") < 0);

  ASSERT_ALWAYS(sys_createAt(ROOT_FD, "test",  Type::REGULAR_FILE) >= 0);
  ASSERT_ALWAYS(sys_createAt(ROOT_FD, "test4", Type::REGULAR_FILE) >= 0);
  ASSERT_ALWAYS(sys_createAt(ROOT_FD, "test3", Type::REGULAR_FILE) >= 0);

  {
    auto testfd = sys_openAt(ROOT_FD, "test");
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


    VfsCommand command = {};

    command.opcode      = VfsCommand::Opcode::SEEK;
    command.fd          = testfd;
    command.seek.anchor = Anchor::BEGIN;
    command.seek.offset = OFFSET;
    sys_async_submit(&command);

    command.opcode      = VfsCommand::Opcode::READ;
    command.fd          = testfd;
    command.read.buf    = readBuf;
    command.read.length = sizeof readBuf;
    sys_async_submit(&command);

    result_t result = 0;
    while(sys_async_wait(&result) != -1);
  }
  lsdir(ROOT_FD);

}

void test2()
{
  rt::log("Triggering page fault...\n");
  const_cast<char*>("str")[0] = 'a';

  rt::log("Exiting...\n");
  _exit(127);
  rt::log("After exiting...\n");

  auto serial1fd = sys_createAt(ROOT_FD, "serial1", Type::REGULAR_FILE);
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

void testCommon()
{
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
}

extern "C" void main()
{
  testCommon();
  switch(sys_fork())
  {
  case 0:
    rt::log("Child process\n");
    test1();
    break;
  default:
    rt::log("Parent process\n");
    test2();
    break;
  }
}
