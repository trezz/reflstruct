#include "reflstruct_test.h"

#include "doctest/doctest.h"
#include "reflstruct.h"

using namespace trezz;

TEST_CASE("reflstruct member functions")
{
    reflstruct person{
        reflmember<int, "age", "json:required">{ 42 },
        reflmember<std::string, "name", "envconfig:name=NAME">{},
    };

    CHECK(person.nb_members == 2);
    CHECK(person.get<"age">() == 42);
    CHECK(person.get<"name">() == "");
    CHECK(!person.contains("address"));
    CHECK(std::string(person.member<"name">().annotation.data) == "envconfig:name=NAME");
    std::string member_names{};
    person.each(
        [&](const auto& member) { member_names += std::string(member.name) + std::string(" "); });
    CHECK(member_names == "age name ");
}
