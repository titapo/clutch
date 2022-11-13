#ifndef CLUTCH_TYPE_ERASED_HH_INCLUDED
#define CLUTCH_TYPE_ERASED_HH_INCLUDED

#include "utility.h"
#include "basic_storage.h"

namespace clutch
{
  namespace detail
  {
    // TODO those are related operations
    template <typename Repr>
    void default_destroy(void* repr)
    {
      delete static_cast<Repr*>(repr);
    }

    template <typename Repr>
    void* default_clone(void* repr)
    {
      return new Repr(*static_cast<Repr*>(repr));
    }

    template <typename Repr>
    void* default_copy_from_value(Repr&& repr)
    {
      return new Repr(static_cast<Repr&&>(repr));
    }

    template <typename Repr, typename... Args>
    void* default_inplace_construct(Args&&... args)
    {
      return new Repr(static_cast<Args&&>(args)...);
    }
  }

  template <typename T>
  struct in_place_t {};

  struct type_erased
  {
    using DestroyFn = void(*)(void *);
    using CloneFn = void *(*)(void *);

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
      : storage(detail::default_copy_from_value(static_cast<Repr>(p_repr)))
      , destroy_fn(detail::default_destroy<Repr>)
      , clone_fn(detail::default_clone<Repr>)
    {
    }

    template <typename Repr, typename... Args>
    type_erased(in_place_t<Repr> tag, Args&&... args)
      : storage(detail::default_inplace_construct<Repr>(static_cast<Args&&>(args)...))
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
