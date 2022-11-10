#define CATCH_CONFIG_MAIN

#include <clutch/type_erased.hh>

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

