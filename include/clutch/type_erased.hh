#ifndef CLUTCH_TYPE_ERASED_HH_INCLUDED
#define CLUTCH_TYPE_ERASED_HH_INCLUDED

#include <cstring> // std::memcpy

namespace clutch
{
  using byte = unsigned char;

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

  template <unsigned StorageSize, typename From>
  void storage_cast_write(byte (&storage)[StorageSize], From&& from)
  {
    static_assert(sizeof(From) == StorageSize);
    std::memcpy(storage, &from, StorageSize);
  }

  template <typename To, unsigned StorageSize>
  To storage_cast_read(const byte (&storage)[StorageSize])
  {
    To to;
    std::memcpy(&to, storage, StorageSize);
    return to;
  }

  template <size_t StorageSize>
  struct basic_storage
  {
    using StorageType = byte[StorageSize];

    StorageType storage;

    basic_storage() = default; // FIXME!!!! remove this or fill with zeros

    template <typename Arg>
    basic_storage(Arg&& arg)
    {
      write(arg);
    }

    void copy_from(basic_storage& other)
    {
      std::memcpy(storage, other.storage, StorageSize);
    }

    template <typename From>
    void write(From&& from)
    {
      static_assert(sizeof(From) == StorageSize);
      std::memcpy(storage, &from, StorageSize);
    }

    template <typename To>
    To read_as() const
    {
      To to;
      std::memcpy(&to, storage, StorageSize);
      return to;
    }
  };

  // inherited because of possible Empty Base Optimization? (It never could be empty :/)
  struct type_erased : basic_storage<sizeof(void*)>
  {
    using DestroyFn = void(*)(void*);
    using CloneFn = void*(*)(void *);

    DestroyFn destroy_fn;
    CloneFn clone_fn;

    void* repr()
    {
      return basic_storage::read_as<void*>();
    }

    void* repr() const
    {
      return basic_storage::read_as<void*>();
    }

    // used for distinguish templatized constructor from copy/move constructors
    struct tag_t {};

    template <typename Repr>
    type_erased(Repr p_repr, tag_t)
      : basic_storage(detail::default_copy_from_value(static_cast<Repr>(p_repr)))
      , destroy_fn(detail::default_destroy<Repr>)
      , clone_fn(detail::default_clone<Repr>)
    {
    }

    // copy
    type_erased(const type_erased& other)
      : basic_storage(reinterpret_cast<byte*>(other.clone_fn(other.repr())))
      , destroy_fn(other.destroy_fn)
      , clone_fn(other.clone_fn)
    {
    }

    // move
    type_erased(type_erased&& other)
      : destroy_fn(other.destroy_fn)
      , clone_fn(other.clone_fn)
    {
      basic_storage::copy_from(other);
      other.basic_storage::write(nullptr);
    }

    ~type_erased()
    {
      destroy_fn(repr());
    }

    type_erased& operator=(const type_erased& other)
    {
      // TODO self assignment check?
      destroy_fn(repr());
      basic_storage::write(other.clone_fn(other.repr()));
      destroy_fn = other.destroy_fn;
      clone_fn = other.clone_fn;
      return *this;
    }

    type_erased& operator=(type_erased&& other)
    {
      // TODO self assignment check?
      destroy_fn(repr());
      basic_storage::copy_from(other);
      destroy_fn = other.destroy_fn;
      clone_fn = other.clone_fn;
      other.basic_storage::write(nullptr);
      return *this;
    }
  };


}

#endif
