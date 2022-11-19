#ifndef CLUTCH_UTILITY_H_INCLUDED
#define CLUTCH_UTILITY_H_INCLUDED

namespace clutch
{
  namespace detail
  {
    template <bool Condition, typename True, typename False>
    struct conditional
    {
      using type = False;
    };

    template <typename True, typename False>
    struct conditional<true, True, False>
    {
      using type = True;
    };

    template <typename T>
    struct is_const
    {
      static constexpr bool value = false;
    };

    template <typename T>
    struct is_const<const T>
    {
      static constexpr bool value = true;
    };

    template <typename T>
    struct remove_reference
    {
      using type = T;
    };

    template <typename T>
    struct remove_reference<T&>
    {
      using type = T;
    };

    template <typename T>
    struct remove_reference<T&&>
    {
      using type = T;
    };

  }
  // All of these are experimental and very likely be changed

  template <typename MemFn>
  struct erased_member_function;

  template <typename R, typename C, typename...Args>
  struct erased_member_function<R (C::*)(Args...)>
  {
    using type = R (*)(void*, Args...);
  };

  template <typename R, typename C, typename...Args>
  struct erased_member_function<R (C::*)(Args...) const>
  {
    using type = R (*)(const void*, Args...);
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

    template <typename R, typename C, typename...Args>
    struct mem_fn_trait<R (C::*)(Args...) const>
    {
      using type = const C;
      using return_type = R;
    };

    template <auto Fn, typename... Args>
    struct forward_function_call_helper
    {
      using repr_ptr_t = typename detail::conditional<
        detail::is_const<typename mem_fn_trait<decltype(Fn)>::type>::value,
        const void*,
        void*>::type;

      static auto call(repr_ptr_t repr, Args... args) -> typename mem_fn_trait<decltype(Fn)>::return_type
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

  template <auto Fn, typename R, typename C, typename... Args>
  struct erase_mem_fn<Fn, R (C::*)(Args...) const> : detail::forward_function_call_helper<Fn, Args...> {};

  namespace detail
  {
    template <typename MemFn>
    struct fn_trait;

    template <typename R, typename...Args>
    struct fn_trait<R (*)(Args...)>
    {
      using return_type = R;
    };

    template <auto Fn, typename Arg0, typename... Args>
    struct erase_fn_helper
    {
      // TODO
      // value, &, &&, *
      // const|non-const
      using repr_ptr_t = typename detail::conditional<
        detail::is_const<typename detail::remove_reference<Arg0>::type>::value,
        const void*,
        void*>::type;

      static auto call(repr_ptr_t repr, Args... args) -> typename fn_trait<decltype(Fn)>::return_type
      {
        using T = typename detail::remove_reference<Arg0>::type;
        return Fn(*static_cast<T*>(repr), static_cast<Args>(args)...);
      }
    };
  }

  template <auto Fn, typename FnType = decltype(Fn)>
  struct erase_fn;

  template <auto Fn, typename R, typename... Args>
  struct erase_fn<Fn, R(*)(Args...)> : detail::erase_fn_helper<Fn, Args...> {};

}

#endif
