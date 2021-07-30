#pragma once

#include <sys/Types.hpp>

// TODO: Figure out a method to handle data dependency between command
//
// This is not compatible with x32 abi, figure out a way around that
struct MountCommandData
{
  const char* mountpoint;
  const char* mountableName;
  const char* arg;
};

struct UmountCommandData
{
  const char* mountpoint;
};

struct OpenCommandData
{
  const char* path;
};

struct CreateCommandData
{
  const char* path;
  Type type;
};

struct LinkCommandData
{
  const char* path;
  const char* target;
};

struct UnlinkCommandData
{
  const char* path;
};

struct ReaddirCommandData
{
  char* buf;
  size_t length;
};

struct SeekCommandData
{
  Anchor anchor;
  off_t offset;
};

struct ReadCommandData
{
  char* buf;
  size_t length;
};

struct WriteCommandData
{
  const char* buf;
  size_t length;
};

struct ResizeCommandData
{
  size_t size;
};

struct CloseCommandData
{
  // Empty
};

struct VfsCommand
{
  enum class Opcode : uword_t // Just picking a resonable sized unsign integer to matina abi compatibility
  {
    MOUNT,
    UMOUNT,

    OPEN,
    CREATE,

    LINK,
    UNLINK,

    READDIR,

    SEEK,
    READ,
    WRITE,
    RESIZE,

    CLOSE,
  };

  Opcode opcode;
  fd_t fd;
  union
  {
    MountCommandData mount;
    UmountCommandData umount;
    OpenCommandData open;
    CreateCommandData create;
    LinkCommandData link;
    UnlinkCommandData unlink;
    ReaddirCommandData readdir;
    SeekCommandData seek;
    ReadCommandData read;
    WriteCommandData write;
    ResizeCommandData resize;
    CloseCommandData close;
  };
};

constexpr static fd_t ROOT_FD = -1;
using vfs_command_t = word_t;
