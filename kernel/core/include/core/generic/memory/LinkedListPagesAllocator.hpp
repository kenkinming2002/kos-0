#pragma once

#include <core/generic/memory/Pages.hpp>

#include <optional>
#include <utils/containers/List.hpp>

namespace core::memory
{
  class LinkedListPagesAllocator
  {
  public:
    std::optional<Pages> allocate(size_t count);
    void deallocate(Pages target);

  public:
    void markAsAvailable(Pages target);
    void markAsUsed(Pages target);

  public:
    const auto& list() const { return m_pagesList; }

  private:
    utils::containers::List<Pages> m_pagesList;
  };
}
