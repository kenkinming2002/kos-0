#pragma once

namespace traits
{
  /** is_same_allocator
   *  
   *  True only if memory allocated by one allocator can be deallocated by other
   *  allocator and vice versa
   *
   */
  template<typename AllocatorT, typename AllocatorU>
  struct is_same_allocator : std::false_type {};

  template<typename AllocatorT, typename AllocatorU>
  using is_same_allocator_v = is_same_allocator<AllocatorT, AllocatorU>::value;
}
