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
    void* default_clone(const void* repr, const heap_allocator& allocator)
    {
      return default_construct<Repr>(allocator, *static_cast<const Repr*>(repr));
    }
  }

  template <typename T>
  struct in_place_t {};

  struct type_erased
  {
    using DestroyFn = void(*)(void *, const detail::heap_allocator&);
    using CloneFn = void *(*)(const void *, const detail::heap_allocator&);

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

  template <size_t StorageSize = 8>
  struct buffered_type_erased
  {
    using StorageType = basic_storage<StorageSize>;

    using DestroyFn = void(*)(void *);
    using CloneFn = void(*)(const void *, StorageType&);

    template <typename Repr, typename... Args>
    static void buffered_construct(StorageType& storage, Args&&... args)
    {
      // TODO Should alignment be considered here
      static_assert(sizeof(typename std::remove_reference<Repr>::type) <= StorageSize);
      void* ptr = storage.data();
      new (ptr) Repr(static_cast<Args&&>(args)...);
    }

    // TODO those are related operations
    template <typename Repr>
    static void buffered_destroy(void* repr)
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
      return storage.data();
    }

    const void* repr() const
    {
      return storage.data();
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
    storage.copy_from(other.storage);
    other.storage.write(nullptr);
  }

  template <size_t StorageSize>
  buffered_type_erased<StorageSize>& buffered_type_erased<StorageSize>::operator=(const buffered_type_erased<StorageSize>& other)
  {
    // TODO self assignment check?
    destroy_fn(repr());
    other.clone_fn(other.repr(), storage);
    destroy_fn = other.destroy_fn;
    clone_fn = other.clone_fn;
    return *this;
  }

  template <size_t StorageSize>
  buffered_type_erased<StorageSize>& buffered_type_erased<StorageSize>::operator=(buffered_type_erased<StorageSize>&& other)
  {
    // TODO self assignment check?
    destroy_fn(repr());
    storage.copy_from(other.storage);
    destroy_fn = other.destroy_fn;
    clone_fn = other.clone_fn;
    other.storage.write(nullptr);
    return *this;
  }

  template <size_t StorageSize>
  buffered_type_erased<StorageSize>::~buffered_type_erased()
  {
    destroy_fn(repr());
  }

}

#endif
