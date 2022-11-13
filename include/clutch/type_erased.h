#ifndef CLUTCH_TYPE_ERASED_HH_INCLUDED
#define CLUTCH_TYPE_ERASED_HH_INCLUDED

#include "utility.h"
#include "basic_storage.h"
#include <cstdlib>

namespace clutch
{
  namespace detail
  {
    struct heap_allocator
    {
      void* allocate(size_t n) const
      {
        return std::malloc(n);
      }

      void deallocate(void *ptr) const
      {
        std::free(ptr);
      }
    };

    template <typename Repr, typename... Args>
    void* default_construct(const heap_allocator& allocator, Args&&... args)
    {
      void* ptr = allocator.allocate(sizeof(Repr));
      return new (ptr) Repr(static_cast<Args&&>(args)...);
    }

    // TODO those are related operations
    template <typename Repr>
    void default_destroy(void* repr, const heap_allocator& allocator)
    {
      static_cast<Repr*>(repr)->~Repr();
      allocator.deallocate(repr);
    }

    template <typename Repr>
    void* default_clone(void* repr, const heap_allocator& allocator)
    {
      return default_construct<Repr>(allocator, *static_cast<Repr*>(repr));
    }
  }

  template <typename T>
  struct in_place_t {};

  struct type_erased
  {
    using DestroyFn = void(*)(void *, const detail::heap_allocator&);
    using CloneFn = void *(*)(void *, const detail::heap_allocator&);

    basic_storage<sizeof(void*)> storage;
    DestroyFn destroy_fn;
    CloneFn clone_fn;

    void* repr()
    {
      return storage.read_as<void*>();
    }

    void* repr() const
    {
      return storage.read_as<void*>();
    }

    // used for distinguish templatized constructor from copy/move constructors
    struct tag_t {};

    template <typename Repr>
    type_erased(Repr p_repr, tag_t)
      : storage(detail::default_construct<Repr>(detail::heap_allocator{}, static_cast<Repr>(p_repr)))
      , destroy_fn(detail::default_destroy<Repr>)
      , clone_fn(detail::default_clone<Repr>)
    {
    }

    template <typename Repr, typename... Args>
    type_erased(in_place_t<Repr> tag, Args&&... args)
      : storage(detail::default_construct<Repr>(detail::heap_allocator{}, static_cast<Args&&>(args)...))
      , destroy_fn(detail::default_destroy<Repr>)
      , clone_fn(detail::default_clone<Repr>)
    {
    }

    type_erased(const type_erased& other);
    type_erased(type_erased&& other);

    type_erased& operator=(const type_erased& other);
    type_erased& operator=(type_erased&& other);

    ~type_erased();
  };


}

#endif
