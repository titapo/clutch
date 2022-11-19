
#include <clutch/type_erased.h>

// copy
clutch::type_erased::type_erased(const type_erased& other)
  : storage(reinterpret_cast<byte*>(other.clone_fn(other.repr(), detail::heap_allocator{})))
  , destroy_fn(other.destroy_fn)
  , clone_fn(other.clone_fn)
{
}

// move
clutch::type_erased::type_erased(type_erased&& other)
  : destroy_fn(other.destroy_fn)
  , clone_fn(other.clone_fn)
{
  storage.copy_from(other.storage);
  other.storage.write(nullptr);
}

clutch::type_erased& clutch::type_erased::operator=(const type_erased& other)
{
  // TODO self assignment check?
  destroy_fn(repr(), detail::heap_allocator{});
  storage.write(other.clone_fn(other.repr(), detail::heap_allocator{}));
  destroy_fn = other.destroy_fn;
  clone_fn = other.clone_fn;
  return *this;
}

clutch::type_erased& clutch::type_erased::operator=(type_erased&& other)
{
  // TODO self assignment check?
  destroy_fn(repr(), detail::heap_allocator{});
  storage.copy_from(other.storage);
  destroy_fn = other.destroy_fn;
  clone_fn = other.clone_fn;
  other.storage.write(nullptr);
  return *this;
}

clutch::type_erased::~type_erased()
{
  destroy_fn(repr(), detail::heap_allocator{});
}
