#ifndef CLUTCH_UTILITY_H_INCLUDED
#define CLUTCH_UTILITY_H_INCLUDED

namespace clutch
{
  // All of these are experimental and very likely be changed

  template <typename MemFn>
  struct erased_member_function;

  template <typename R, typename C, typename...Args>
  struct erased_member_function<R (C::*)(Args...)>
  {
    using type = R (*)(void*, Args...);
  };

  namespace detail
  {
    template <typename MemFn>
    struct mem_fn_trait;

    template <typename R, typename C, typename...Args>
    struct mem_fn_trait<R (C::*)(Args...)>
    {
      using type = C;
      using return_type = R;
    };

    template <auto Fn, typename... Args>
    struct forward_function_call_helper
    {
      static auto call(void* repr, Args... args) -> typename mem_fn_trait<decltype(Fn)>::return_type
      {
        using T = typename mem_fn_trait<decltype(Fn)>::type;
        return (static_cast<T*>(repr)->*Fn)(static_cast<Args>(args)...);
      }
    };
  }

  template <auto Fn, typename FnType = decltype(Fn)>
  struct erase_mem_fn;

  template <auto Fn, typename R, typename C, typename... Args>
  struct erase_mem_fn<Fn, R (C::*)(Args...)> : detail::forward_function_call_helper<Fn, Args...> {};

}

#endif
