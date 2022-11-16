#include "doctest/doctest.h"
#include "envconfig.h"
#include "reflstruct.h"

#include <functional>
#include <map>
#include <type_traits>

using namespace trezz;

static_assert(envconfig::is_invalid_annotation<"">() == 0);
static_assert(envconfig::is_invalid_annotation<"json:required">() == 0);
static_assert(envconfig::is_invalid_annotation<"envconfig:required">() == 0);
static_assert(envconfig::is_invalid_annotation<"envconfig:required,ignore">() == 0);
static_assert(envconfig::is_invalid_annotation<"envconfig:default">() == 1);
static_assert(envconfig::is_invalid_annotation<"envconfig:ignore,name=MYNAME">() == 0);
static_assert(envconfig::is_invalid_annotation<"envconfig:required,unknown,ignore">() == 2);

TEST_CASE("envconfig::process nominal")
{
    std::map<std::string, std::string> env{
        { "NAME", "Alice" },
        { "AGE", "51" },
        { "MY_NAME", "Bob" },
        { "BIRTHDAY", "12" },
    };

    reflstruct person{
        reflmember<int, "age", "envconfig:ignore">{ 42 },
        reflmember<std::string, "name", "envconfig:name=MY_NAME,required">{},
        reflmember<int, "birthday">{ 0 },
    };

    auto env_getter = [&](const char* name) -> char* {
        if (!env.contains(std::string(name))) {
            return nullptr;
        }
        return env[std::string(name)].data();
    };

    CHECK_NOTHROW(envconfig::detail::process(person, env_getter));
    CHECK(person.get<"name">() == "Bob");
    CHECK(person.get<"age">() == 42);
    CHECK(person.get<"birthday">() == 12);

    env.erase("MY_NAME");
    CHECK_THROWS_WITH(envconfig::detail::process(person, env_getter),
                      "required 'MY_NAME' not found");
}

static std::map<std::string, std::string> env{
    { "NAME", "Alice" },
    { "AGE", "51" },
    { "MY_NAME", "Bob" },
    { "BIRTHDAY", "12" },
};

static char* env_getter(const char* name)
{
    if (!env.contains(std::string(name))) {
        return nullptr;
    }
    return env[std::string(name)].data();
};

struct Person
{
    int age{};
    std::string name{};
    int birthday{};

    TREZZ_REFLSTRUCT_BEGIN(Person)
    TREZZ_REFLMEMBER(age, "envconfig:ignore")
    TREZZ_REFLMEMBER(name, "envconfig:name=MY_NAME,required")
    TREZZ_REFLMEMBER(birthday, "")
    TREZZ_REFLSTRUCT_END
};

TEST_CASE("envconfig::process in struct")
{
    Person person{
        .age = 42,
        .name = "Alice",
        .birthday = 12,
    };

    CHECK_NOTHROW(envconfig::detail::process(person, env_getter));
    CHECK(person.name == "Bob");
    CHECK(person.age == 42);
    CHECK(person.birthday == 12);

    env.erase("MY_NAME");
    CHECK_THROWS_WITH(envconfig::detail::process(person, env_getter),
                      "required 'MY_NAME' not found");

    // Ensure const reflection works.
    const Person p2{};
    const trezz::reflstruct r2 = Person::make_trezz_reflstruct(p2);
    std::ignore = r2;
}
