#include "envconfig_test.h"

#include "doctest/doctest.h"
#include "envconfig.h"

#include <map>

using namespace trezz;

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

struct Person
{
    int age{ 42 };
    std::string name{};
    int birthday{ 0 };
};

auto reflect(Person& p)
{
    return reflstruct{
        reflmember<int&, "age", "envconfig:ignore">{ p.age },
        reflmember<std::string&, "name", "envconfig:name=MY_NAME,required">{ p.name },
        reflmember<int&, "birthday">{ p.birthday },
    };
};

TEST_CASE("envconfig::process in struct")
{
    std::map<std::string, std::string> env{
        { "NAME", "Alice" },
        { "AGE", "51" },
        { "MY_NAME", "Bob" },
        { "BIRTHDAY", "12" },
    };

    auto env_getter = [&](const char* name) -> char* {
        if (!env.contains(std::string(name))) {
            return nullptr;
        }
        return env[std::string(name)].data();
    };

    Person person{};
    auto reflperson = reflect(person);

    CHECK_NOTHROW(envconfig::detail::process(reflperson, env_getter));
    CHECK(person.name == "Bob");
    CHECK(person.age == 42);
    CHECK(person.birthday == 12);

    env.erase("MY_NAME");
    CHECK_THROWS_WITH(envconfig::detail::process(reflperson, env_getter),
                      "required 'MY_NAME' not found");
}
