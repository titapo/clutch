#ifndef CLUTCH_TYPE_ERASED_HH_INCLUDED
#define CLUTCH_TYPE_ERASED_HH_INCLUDED

#include "utility.h"
#include "basic_storage.h"
#include <cstdlib>
#include <utility>
#include <iostream>

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
  }

  struct heap_storage
  {
    static constexpr size_t StorageSize = sizeof(void*);
    using StorageType = basic_storage<StorageSize>;

    void* read()
    {
      return storage.read_as<void*>();
    }

    const void* read() const
    {
      return storage.read_as<void*>();
    }

    template <typename Repr, typename... Args>
    static void construct(heap_storage& storage, Args&&... args)
    {
      void* ptr = detail::heap_allocator{}.allocate(sizeof(Repr));
      new (ptr) Repr(static_cast<Args&&>(args)...);
      storage.storage.write(ptr);
    }

    // TODO those are related operations
    template <typename Repr>
    static void destroy(void* repr, heap_storage& storage)
    {
      static_cast<Repr*>(repr)->~Repr();
      detail::heap_allocator{}.deallocate(repr);
      storage.storage.write(nullptr);
    }

    template <typename Repr>
    static void clone(const void* repr, heap_storage& storage)
    {
      return construct<Repr>(storage, *static_cast<const Repr*>(repr));
    }


    StorageType storage;
  };

  template <size_t StorageSize>
  struct fixed_storage
  {
    using StorageType = basic_storage<StorageSize>;

    void* read()
    {
      return storage.data();
    }

    const void* read() const
    {
      return storage.data();
    }

    template <typename Repr, typename... Args>
    static void construct(fixed_storage<StorageSize>& storage, Args&&... args)
    {
      // TODO Should alignment be considered here
      static_assert(sizeof(typename std::remove_reference<Repr>::type) <= StorageSize);
      void* ptr = storage.storage.data();
      new (ptr) Repr(static_cast<Args&&>(args)...);
    }

    // TODO those are related operations
    template <typename Repr>
    static void destroy(void* repr, fixed_storage<StorageSize>& storage)
    {
      static_cast<Repr*>(repr)->~Repr();
    }

    template <typename Repr>
    static void clone(const void* repr, fixed_storage<StorageSize>& storage)
    {
      return construct<Repr>(storage, *static_cast<const Repr*>(repr));
    }

    StorageType storage;
  };

  template <typename T>
  struct in_place_t {};

  template <typename StorageType>
  struct basic_erased_type
  {
    using DestroyFn = void(*)(void *, StorageType&);
    using CloneFn = void(*)(const void *, StorageType&);

    StorageType storage;
    DestroyFn destroy_fn;
    CloneFn clone_fn;

    void* repr()
    {
      return storage.read();
    }

    const void* repr() const
    {
      return storage.read();
    }

    // used for distinguish templatized constructor from copy/move constructors
    struct tag_t {};

    template <typename Repr>
    basic_erased_type(Repr p_repr, tag_t)
      : destroy_fn(StorageType::template destroy<Repr>)
      , clone_fn(&StorageType::template clone<Repr>)
    {
      StorageType::template construct<Repr>(storage, static_cast<Repr>(p_repr));
    }

    template <typename Repr, typename... Args>
    basic_erased_type(in_place_t<Repr> tag, Args&&... args)
      : destroy_fn(StorageType::template destroy<Repr>)
      , clone_fn(StorageType::template clone<Repr>)
    {
      StorageType::template construct<Repr>(storage, static_cast<Args&&>(args)...);
    }

    basic_erased_type(const basic_erased_type& other);
    basic_erased_type(basic_erased_type&& other);

    basic_erased_type& operator=(const basic_erased_type& other);
    basic_erased_type& operator=(basic_erased_type&& other);

    ~basic_erased_type();
  };


  // copy
  template <typename StorageType>
  basic_erased_type<StorageType>::basic_erased_type(const basic_erased_type<StorageType>& other)
    : destroy_fn(other.destroy_fn)
    , clone_fn(other.clone_fn)
  {
    other.clone_fn(other.repr(), storage);
  }

  // move
  template <typename StorageType>
  basic_erased_type<StorageType>::basic_erased_type(basic_erased_type<StorageType>&& other)
    : destroy_fn(other.destroy_fn)
    , clone_fn(other.clone_fn)
  {
    storage.storage.copy_from(other.storage.storage);
    other.storage.storage.write(nullptr);
  }

  template <typename StorageType>
  basic_erased_type<StorageType>& basic_erased_type<StorageType>::operator=(const basic_erased_type<StorageType>& other)
  {
    // TODO self assignment check?
    destroy_fn(repr(), storage);
    other.clone_fn(other.repr(), storage);
    destroy_fn = other.destroy_fn;
    clone_fn = other.clone_fn;
    return *this;
  }

  template <typename StorageType>
  basic_erased_type<StorageType>& basic_erased_type<StorageType>::operator=(basic_erased_type<StorageType>&& other)
  {
    // TODO self assignment check?
    destroy_fn(repr(), storage);
    storage.storage.copy_from(other.storage.storage);
    destroy_fn = other.destroy_fn;
    clone_fn = other.clone_fn;
    other.storage.storage.write(nullptr);
    return *this;
  }

  template <typename StorageType>
  basic_erased_type<StorageType>::~basic_erased_type()
  {
    destroy_fn(repr(), storage);
  }

  using type_erased = basic_erased_type<heap_storage>;

  template <size_t StorageSize>
  using buffered_type_erased = basic_erased_type<fixed_storage<StorageSize>>;

}

#endif
