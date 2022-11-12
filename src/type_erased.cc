
#include <clutch/type_erased.hh>

// copy
clutch::type_erased::type_erased(const type_erased& other)
  : basic_storage(reinterpret_cast<byte*>(other.clone_fn(other.repr())))
  , destroy_fn(other.destroy_fn)
  , clone_fn(other.clone_fn)
{
}

// move
clutch::type_erased::type_erased(type_erased&& other)
  : destroy_fn(other.destroy_fn)
  , clone_fn(other.clone_fn)
{
  basic_storage::copy_from(other);
  other.basic_storage::write(nullptr);
}

clutch::type_erased& clutch::type_erased::operator=(const type_erased& other)
{
  // TODO self assignment check?
  destroy_fn(repr());
  basic_storage::write(other.clone_fn(other.repr()));
  destroy_fn = other.destroy_fn;
  clone_fn = other.clone_fn;
  return *this;
}

clutch::type_erased& clutch::type_erased::operator=(type_erased&& other)
{
  // TODO self assignment check?
  destroy_fn(repr());
  basic_storage::copy_from(other);
  destroy_fn = other.destroy_fn;
  clone_fn = other.clone_fn;
  other.basic_storage::write(nullptr);
  return *this;
}

clutch::type_erased::~type_erased()
{
  destroy_fn(repr());
}

