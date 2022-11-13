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

SCENARIO("type_erased")
{
  REQUIRE(counted::count == 0);
  WHEN("created with a temporary")
  {
    clutch::type_erased te(counted{}, {});
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
      clutch::type_erased te(c1, {});
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
SCENARIO("emplace operations")
{
  GIVEN("...")
  {
    WHEN("in-place constructed")
    {
      auto e = clutch::type_erased(clutch::in_place_t<Complex>{}, 4, 'c', false);
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
SCENARIO("")
{
  clutch::type_erased e(S{5}, {});
  REQUIRE(static_cast<S*>(e.repr())->i == 5);

  WHEN("copied")
  {
    clutch::type_erased e2{e};
    REQUIRE(static_cast<S*>(e.repr())->i == 5);
    REQUIRE(static_cast<S*>(e2.repr())->i == 5);
  }

  WHEN("copy-assigned")
  {
    clutch::type_erased e2{S{20}, {}};
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
    clutch::type_erased e2{S{20}, {}};
    e2 = std::move(e);
    REQUIRE(e.repr() == nullptr);
    REQUIRE(static_cast<S*>(e2.repr())->i == 5);
  }

  WHEN("swapped")
  {
    using std::swap;
    clutch::type_erased e2{S{20}, {}};
    swap(e, e2);
    REQUIRE(static_cast<S*>(e.repr())->i == 20);
    REQUIRE(static_cast<S*>(e2.repr())->i == 5);
  }

}

// Here is the current best way to use it
// There is a lot of opportunity for improvement

class Animal : public clutch::type_erased
{
  public:
    std::string talk(int i)
    {
      return talk_fn(repr(), i);
    }

    using TalkFn = clutch::erased_member_function<decltype(&Animal::talk)>::type;
    TalkFn talk_fn;

    template <typename T>
    Animal(T&& t)
      : type_erased(t, {})
      , talk_fn(&clutch::erase_mem_fn<&T::talk>::call)
    {}

};

struct Cat
{
  std::string talk(int i)
  {
    return "Meow";
  }
};

struct Dog
{
  std::string talk(int i)
  {
    return "Vau";
  }
};

SCENARIO("derived class in action")
{
  GIVEN("a cat")
  {
    Animal animal{Cat{}};

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
    Animal animal{Dog{}};

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
    Animal animal{Cat{}};
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

// TODO emplacement should be supported
