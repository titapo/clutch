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

  template <unsigned StorageSize>
  void storage_cast_write(byte (&storage)[StorageSize], void* repr)
  {
    std::memcpy(storage, &repr, StorageSize);
  }

  template <typename To, unsigned StorageSize>
  To storage_cast_read(const byte (&storage)[StorageSize])
  {
    To to;
    std::memcpy(&to, storage, StorageSize);
    return to;
  }

  struct type_erased
  {
    using DestroyFn = void(*)(void*);
    using CloneFn = void*(*)(void *);

    static constexpr auto StorageSize = sizeof(void*);
    using StorageType = byte[StorageSize];

    //void* repr{nullptr};
    StorageType storage;
    DestroyFn destroy_fn;
    CloneFn clone_fn;

    void* repr()
    {
      return storage_cast_read<void*>(storage);
    }

    void* repr() const
    {
      return storage_cast_read<void*>(storage);
    }

    // used for distinguish templatized constructor from copy/move constructors
    struct tag_t {};

    template <typename Repr>
    type_erased(Repr p_repr, tag_t)
      //: repr(detail::default_copy_from_value(static_cast<Repr>(p_repr)))
      //: storage(reinterpret_cast<StorageType>(detail::default_copy_from_value(static_cast<Repr>(p_repr))))
      : destroy_fn(detail::default_destroy<Repr>)
      , clone_fn(detail::default_clone<Repr>)
    {
      storage_cast_write(storage, detail::default_copy_from_value(static_cast<Repr>(p_repr)));
    }

    // copy
    type_erased(const type_erased& other)
      //: repr(other.clone_fn(other.repr))
      //: storage(reinterpret_cast<byte*>(other.clone_fn(other.repr())))
      : destroy_fn(other.destroy_fn)
      , clone_fn(other.clone_fn)
    {
      storage_cast_write(storage, other.clone_fn(other.repr()));
    }

    // move
    type_erased(type_erased&& other)
      //: storage(other.storage)
      : destroy_fn(other.destroy_fn)
      , clone_fn(other.clone_fn)
    {
      std::memcpy(storage, other.storage, StorageSize);
      //storage_cast_write(storage, s);
      storage_cast_write(other.storage, nullptr);
    }

    ~type_erased()
    {
      destroy_fn(repr());
    }

    type_erased& operator=(const type_erased& other)
    {
      destroy_fn(repr());
      storage_cast_write(storage, other.clone_fn(other.repr()));
      //repr = other.clone_fn(other.repr);
      destroy_fn = other.destroy_fn;
      clone_fn = other.clone_fn;
      return *this;
    }

    type_erased& operator=(type_erased&& other)
    {
      // TODO self assignment check?
      destroy_fn(repr());
      std::memcpy(storage, other.storage, StorageSize);
      //repr = other.repr;
      destroy_fn = other.destroy_fn;
      clone_fn = other.clone_fn;
      //other.repr = nullptr;
      storage_cast_write(other.storage, nullptr);
      return *this;
    }
  };


}

#endif
