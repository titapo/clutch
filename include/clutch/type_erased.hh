#ifndef CLUTCH_TYPE_ERASED_HH_INCLUDED
#define CLUTCH_TYPE_ERASED_HH_INCLUDED

#include "utility.h"

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

    template <typename Repr, typename... Args>
    void* default_inplace_construct(Args&&... args)
    {
      return new Repr(static_cast<Args&&>(args)...);
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
    public:
      using StorageType = byte[StorageSize];

      basic_storage() = default; // FIXME!!!! remove this or fill with zeros

      template <typename Arg>
      basic_storage(Arg&& arg)
      {
        write(arg);
      }

      void copy_from(basic_storage& other)
      {
        std::memcpy(payload, other.payload, StorageSize);
      }

      template <typename From>
      void write(From&& from)
      {
        static_assert(sizeof(From) == StorageSize);
        std::memcpy(payload, &from, StorageSize);
      }

      template <typename To>
      To read_as() const
      {
        To to;
        std::memcpy(&to, payload, StorageSize);
        return to;
      }

    private:
      StorageType payload;
  };

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
