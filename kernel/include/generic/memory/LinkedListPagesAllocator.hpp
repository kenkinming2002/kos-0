#pragma once

#include <generic/memory/Pages.hpp>

#include <librt/containers/List.hpp>
#include <librt/Optional.hpp>

namespace core::memory
{
  class LinkedListPagesAllocator
  {
  public:
    rt::Optional<Pages> allocate(size_t count);
    void deallocate(Pages target);

  public:
    void markAsAvailable(Pages target);
    void markAsUsed(Pages target);

  public:
    const auto& list() const { return m_pagesList; }

  private:
    rt::containers::List<Pages> m_pagesList;
  };
}
