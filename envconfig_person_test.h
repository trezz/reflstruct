#pragma once

#include "reflstruct.h"

#include <string>

using namespace trezz;

namespace test {

struct Person
{
    int age{};
    std::string name{};
    int birthday{};
};

} // namespace test

TREZZ_REFLSTRUCT_BEGIN(test::Person)
TREZZ_REFLMEMBER(age, "envconfig:ignore")
TREZZ_REFLMEMBER(name, "envconfig:name=MY_NAME,required")
TREZZ_REFLMEMBER(birthday, "")
TREZZ_REFLSTRUCT_END
