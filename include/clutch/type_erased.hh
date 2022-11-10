#ifndef CLUTCH_TYPE_ERASED_HH_INCLUDED
#define CLUTCH_TYPE_ERASED_HH_INCLUDED

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
  }

  struct type_erased
  {
    using DestroyFn = void(*)(void*);
    using CloneFn = void*(*)(void *);

    void* repr{nullptr};
    DestroyFn destroy_fn;
    CloneFn clone_fn;

    // used for distinguish templatized constructor from copy/move constructors
    struct tag_t {};

    template <typename Repr>
    type_erased(Repr p_repr, tag_t)
      : repr(detail::default_copy_from_value(static_cast<Repr>(p_repr)))
      , destroy_fn(detail::default_destroy<Repr>)
      , clone_fn(detail::default_clone<Repr>)
    {}

    // copy
    type_erased(const type_erased& other)
      : repr(other.clone_fn(other.repr))
      , destroy_fn(other.destroy_fn)
      , clone_fn(other.clone_fn)
    {}

    // move
    type_erased(type_erased&& other)
      : repr(other.repr)
      , destroy_fn(other.destroy_fn)
      , clone_fn(other.clone_fn)
    {
      other.repr = nullptr;
    }

    ~type_erased()
    {
      destroy_fn(repr);
    }

    type_erased& operator=(const type_erased& other)
    {
      destroy_fn(repr);
      repr = other.clone_fn(other.repr);
      destroy_fn = other.destroy_fn;
      clone_fn = other.clone_fn;
      return *this;
    }

    type_erased& operator=(type_erased&& other)
    {
      destroy_fn(repr);
      repr = other.repr;
      destroy_fn = other.destroy_fn;
      clone_fn = other.clone_fn;
      other.repr = nullptr;
      return *this;
    }
  };


}

#endif
