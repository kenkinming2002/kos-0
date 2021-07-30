#pragma once

#include <sys/VfsCommand.hpp>

#include <librt/containers/StaticVector.hpp>
#include <librt/Optional.hpp>

namespace core::vfs
{
  /* Not preserved across fork */
  class CommandQueue
  {
  public:
    static constexpr size_t MAX_COMMAND_COUNT = 32;

  public:
    vfs_command_t submit(VfsCommand command)
    {
      for(size_t i=0; i<MAX_COMMAND_COUNT; ++i)
        if(!commands[i])
        {
          commands[i] = command;
          return i;
        }

      return -1;
    }

    vfs_command_t retrive(VfsCommand& command)
    {
      for(size_t i=0; i<MAX_COMMAND_COUNT; ++i)
        if(commands[i])
        {
          command = *rt::exchange(commands[i], rt::nullOptional);
          return i;
        }

      return -1;
    }

  private:
    // We could pack this better, but this is gonna change soon anyway
    rt::Optional<VfsCommand> commands[MAX_COMMAND_COUNT];
  };
}
