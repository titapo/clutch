#ifndef CLUTCH_BASIC_STORAGE_H_INCLUDED
#define CLUTCH_BASIC_STORAGE_H_INCLUDED

#include <cstring> // std::memcpy

namespace clutch
{
  using byte = unsigned char;

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

      void* data()
      {
        return payload;
      }

      const void* data() const
      {
        return payload;
      }

    private:
      StorageType payload;
  };
}

#endif
