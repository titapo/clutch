#define CATCH_CONFIG_MAIN

#include <clutch/type_erased.h>

#include <catch2/catch.hpp>

struct counted
{
  inline static int count{0};
  counted() { ++count; }
  counted(const counted&) { ++count; }
  counted(counted&&) { ++count; }
  ~counted() { --count; }
};

TEMPLATE_TEST_CASE("type_erased", "", clutch::type_erased, clutch::buffered_type_erased<1>)
{
  using TypeErased = TestType;
  REQUIRE(counted::count == 0);
  WHEN("created with a temporary")
  {
    TypeErased te(counted{});
    THEN("a single instance is held")
    {
      REQUIRE(counted::count == 1);
    }
  }
  WHEN("Given an lvalue")
  {
    counted c1;
    REQUIRE(counted::count == 1);
    WHEN("type_erased created from it")
    {
      TypeErased te(c1);
      THEN("another instance is held")
      {
        REQUIRE(counted::count == 2);
      }
    }
    REQUIRE(counted::count == 1);
  }
  REQUIRE(counted::count == 0);
}

struct Complex
{
  Complex(int i, char c, bool b)
    : i(i)
    , c(c)
    , b(b)
  {}

  int i{-2};
  char c{'n'};
  bool b{true};
};

// NOTE: direct access to repr is considered as a bad practice and should be impossible
TEMPLATE_TEST_CASE("emplace operations", "", clutch::type_erased, clutch::buffered_type_erased<8>)
{
  using TypeErased = TestType;
  GIVEN("...")
  {
    WHEN("in-place constructed")
    {
      auto e = TypeErased(clutch::in_place_t<Complex>{}, 4, 'c', false);
      THEN("contains proper values")
      {
        REQUIRE(static_cast<Complex*>(e.repr())->i == 4);
        REQUIRE(static_cast<Complex*>(e.repr())->c == 'c');
        REQUIRE(static_cast<Complex*>(e.repr())->b == false);
      }
    }
  }
}

// TODO .emplace for "operator=(in_place)"

struct S
{
  int i{-1};
};

// NOTE: direct access to repr is considered as a bad practice
TEMPLATE_TEST_CASE("transfer operations", "", clutch::type_erased, clutch::buffered_type_erased<16>)
{
  clutch::type_erased e(S{5});
  REQUIRE(static_cast<S*>(e.repr())->i == 5);

  WHEN("copied")
  {
    clutch::type_erased e2{e};
    REQUIRE(static_cast<S*>(e.repr())->i == 5);
    REQUIRE(static_cast<S*>(e2.repr())->i == 5);
  }

  WHEN("copy-assigned")
  {
    clutch::type_erased e2{S{20}};
    e2 = e;
    REQUIRE(static_cast<S*>(e.repr())->i == 5);
    REQUIRE(static_cast<S*>(e2.repr())->i == 5);
  }

  WHEN("moved")
  {
    clutch::type_erased e2{std::move(e)};
    REQUIRE(e.repr() == nullptr);
    REQUIRE(static_cast<S*>(e2.repr())->i == 5);
  }

  WHEN("move-assigned")
  {
    clutch::type_erased e2{S{20}};
    e2 = std::move(e);
    REQUIRE(e.repr() == nullptr);
    REQUIRE(static_cast<S*>(e2.repr())->i == 5);
  }

  WHEN("swapped")
  {
    using std::swap;
    clutch::type_erased e2{S{20}};
    swap(e, e2);
    REQUIRE(static_cast<S*>(e.repr())->i == 20);
    REQUIRE(static_cast<S*>(e2.repr())->i == 5);
  }
}


// Here is the current best way to use it
// There is a lot of opportunity for improvement

template <typename TypeErasedBase>
class Animal : public TypeErasedBase
{
  public:
    std::string talk(int i) const
    {
      return talk_fn(TypeErasedBase::repr(), i);
    }

    std::string eat(double d)
    {
      return eat_fn(TypeErasedBase::repr(), d);
    }

    using TalkFn = typename clutch::erased_member_function<decltype(&Animal<TypeErasedBase>::talk)>::type;
    using EatFn = typename clutch::erased_member_function<decltype(&Animal<TypeErasedBase>::eat)>::type;

    TalkFn talk_fn;
    EatFn eat_fn;

    template <typename T>
    Animal(T&& t)
      : TypeErasedBase(t)
      , talk_fn(&clutch::erase_mem_fn<&T::talk>::call)
      , eat_fn(&clutch::erase_mem_fn<&T::eat>::call)
    {}

};

struct Cat
{
  std::string talk(int i) const
  {
    return "Meow";
  }

  std::string eat(double d)
  {
    return "catfood";
  }
};

struct Dog
{
  std::string talk(int i) const
  {
    return "Vau";
  }

  std::string eat(double d)
  {
    return "dogfood";
  }
};

// TODO split this test based on behaviour
TEMPLATE_TEST_CASE("derived class in action", "", clutch::type_erased, clutch::buffered_type_erased<8>)
{
  using MyAnimal = Animal<TestType>;
  GIVEN("a cat")
  {
    MyAnimal animal{Cat{}};

    WHEN("it talks")
    {
      const auto out = animal.talk(3);

      THEN("it returned: Meow")
      {
        REQUIRE(out == "Meow");
      }
    }
  }

  GIVEN("a dog")
  {
    MyAnimal animal{Dog{}};

    WHEN("it talks")
    {
      const auto out = animal.talk(3);

      THEN("it returned: Vau")
      {
        REQUIRE(out == "Vau");
      }
    }
  }

  GIVEN("a cat as an animal")
  {
    MyAnimal animal{Cat{}};
    WHEN("it is overridden by a dog")
    {
      animal = Dog{};
      THEN("it talks as a dog")
      {
        REQUIRE(animal.talk(2) == "Vau");
      }
    }
  }
}

struct Button
{
  Button(const std::string& title)
    : title{title}
  {}

  std::string title{};
};

std::string foo(const Button& button, int i)
{
  return "Button[" + button.title + "]";
}



template <typename TypeErasedBase>
class Widget : public TypeErasedBase
{
  public:
    std::string foo(int i) const
    {
      return foo_fn(TypeErasedBase::repr(), i);
    }

    //std::string bar(double d)
    //{
    //  return bar_fn(TypeErasedBase::repr(), d);
    //}

    using FooFn = typename clutch::erased_member_function<decltype(&Widget<TypeErasedBase>::foo)>::type;
    //using BarFn = typename clutch::erased_member_function<decltype(&Widget<TypeErasedBase>::bar)>::type;

    FooFn foo_fn;
    //BarFn bar_fn;

    template <typename T, typename... Args>
    Widget(clutch::in_place_t<T> tag, Args&&... args)
      : TypeErasedBase(tag, static_cast<Args>(args)...)
      , foo_fn(&clutch::erase_fn<static_cast<std::string(*)(const T&,int)>(&::foo)>::call)
      //, bar_fn(&clutch::erase_fn<&bar>::call)
    {}

    /*
    template <typename T>
    Widget(T&& t)
      : TypeErasedBase(t)
      , foo_fn(&clutch::erase_fn<&foo>::call)
      //, bar_fn(&clutch::erase_fn<&bar>::call)
    {}
    */

};

// SCENARIO erase_function works with free functions
TEMPLATE_TEST_CASE("erase_function works with free function", "", clutch::type_erased, clutch::buffered_type_erased<32>)
{
  using MyWidget = Widget<TestType>;
  MyWidget mywidget{clutch::in_place_t<Button>{}, "hello"};
  REQUIRE(mywidget.foo(4) == "Button[hello]");

}


// TODO emplacement should be supported
