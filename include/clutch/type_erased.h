#ifndef CLUTCH_TYPE_ERASED_HH_INCLUDED
#define CLUTCH_TYPE_ERASED_HH_INCLUDED

#include "utility.h"
#include "basic_storage.h"
#include <cstdlib>
#include <utility>
#include <iostream>

namespace clutch
{

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

    StorageType storage;
  };

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
    void default_construct(heap_storage& storage, Args&&... args)
    {
      void* ptr = heap_allocator{}.allocate(sizeof(Repr));
      new (ptr) Repr(static_cast<Args&&>(args)...);
      storage.storage.write(ptr);
    }

    // TODO those are related operations
    template <typename Repr>
    void default_destroy(void* repr, heap_storage& storage)
    {
      static_cast<Repr*>(repr)->~Repr();
      heap_allocator{}.deallocate(repr);
      storage.storage.write(nullptr);
    }

    template <typename Repr>
    void default_clone(const void* repr, heap_storage& storage)
    {
      return default_construct<Repr>(storage, *static_cast<const Repr*>(repr));
    }

  }

  template <typename T>
  struct in_place_t {};

  struct type_erased
  {
    using StorageType = heap_storage;

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
    type_erased(Repr p_repr, tag_t)
      : destroy_fn(detail::default_destroy<Repr>)
      , clone_fn(detail::default_clone<Repr>)
    {
      detail::default_construct<Repr>(storage, static_cast<Repr>(p_repr));
    }

    template <typename Repr, typename... Args>
    type_erased(in_place_t<Repr> tag, Args&&... args)
      : destroy_fn(detail::default_destroy<Repr>)
      , clone_fn(detail::default_clone<Repr>)
    {
      detail::default_construct<Repr>(storage, static_cast<Args&&>(args)...);
    }

    type_erased(const type_erased& other);
    type_erased(type_erased&& other);

    type_erased& operator=(const type_erased& other);
    type_erased& operator=(type_erased&& other);

    ~type_erased();
  };

  template <size_t StorageSize = 8>
  struct buffered_type_erased
  {
    using StorageType = fixed_storage<StorageSize>;

    using DestroyFn = void(*)(void *, StorageType&);
    using CloneFn = void(*)(const void *, StorageType&);

    template <typename Repr, typename... Args>
    static void buffered_construct(StorageType& storage, Args&&... args)
    {
      // TODO Should alignment be considered here
      static_assert(sizeof(typename std::remove_reference<Repr>::type) <= StorageSize);
      void* ptr = storage.storage.data();
      new (ptr) Repr(static_cast<Args&&>(args)...);
    }

    // TODO those are related operations
    template <typename Repr>
    static void buffered_destroy(void* repr, StorageType& storage)
    {
      static_cast<Repr*>(repr)->~Repr();
    }

    template <typename Repr>
    static void buffered_clone(const void* repr, StorageType& storage)
    {
      return buffered_construct<Repr>(storage, *static_cast<const Repr*>(repr));
    }

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
    buffered_type_erased(Repr p_repr, tag_t)
      : destroy_fn(buffered_destroy<Repr>)
      , clone_fn(&buffered_clone<Repr>)
    {
      buffered_construct<Repr>(storage, static_cast<Repr>(p_repr));
    }

    template <typename Repr, typename... Args>
    buffered_type_erased(in_place_t<Repr> tag, Args&&... args)
      : destroy_fn(buffered_destroy<Repr>)
      , clone_fn(buffered_clone<Repr>)
    {
      buffered_construct<Repr>(storage, static_cast<Args&&>(args)...);
    }

    buffered_type_erased(const buffered_type_erased& other);
    buffered_type_erased(buffered_type_erased&& other);

    buffered_type_erased& operator=(const buffered_type_erased& other);
    buffered_type_erased& operator=(buffered_type_erased&& other);

    ~buffered_type_erased();
  };


  // copy
  template <size_t StorageSize>
  buffered_type_erased<StorageSize>::buffered_type_erased(const buffered_type_erased<StorageSize>& other)
    : destroy_fn(other.destroy_fn)
    , clone_fn(other.clone_fn)
  {
    other.clone_fn(other.repr(), storage);
  }

  // move
  template <size_t StorageSize>
  buffered_type_erased<StorageSize>::buffered_type_erased(buffered_type_erased<StorageSize>&& other)
    : destroy_fn(other.destroy_fn)
    , clone_fn(other.clone_fn)
  {
    storage.storage.copy_from(other.storage.storage);
    other.storage.storage.write(nullptr);
  }

  template <size_t StorageSize>
  buffered_type_erased<StorageSize>& buffered_type_erased<StorageSize>::operator=(const buffered_type_erased<StorageSize>& other)
  {
    // TODO self assignment check?
    destroy_fn(repr(), storage);
    other.clone_fn(other.repr(), storage);
    destroy_fn = other.destroy_fn;
    clone_fn = other.clone_fn;
    return *this;
  }

  template <size_t StorageSize>
  buffered_type_erased<StorageSize>& buffered_type_erased<StorageSize>::operator=(buffered_type_erased<StorageSize>&& other)
  {
    // TODO self assignment check?
    destroy_fn(repr(), storage);
    storage.storage.copy_from(other.storage.storage);
    destroy_fn = other.destroy_fn;
    clone_fn = other.clone_fn;
    other.storage.storage.write(nullptr);
    return *this;
  }

  template <size_t StorageSize>
  buffered_type_erased<StorageSize>::~buffered_type_erased()
  {
    destroy_fn(repr(), storage);
  }

}

#endif
