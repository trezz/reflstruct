#pragma once

#include "reflstruct.h"

namespace trezz::test {

static constexpr reflstruct person{
    reflmember<std::string_view, "name">{ "Alice" },
    reflmember<size_t, "age">{ 42 },
};

static_assert(person.get<"name">() == "Alice");
static_assert(person.get<"age">() == 42);

static_assert(person.contains("name"));
static_assert(!person.contains("toto"));

inline constexpr auto anno = detail::string_literal{ R"(
    json:req,required,omitempty
    envconfig:required=true,name=example_name,default_value=
)" };

static_assert(annotation::has<anno, "json">());
static_assert(annotation::has<anno, "envconfig">());
static_assert(!annotation::has<anno, "protobuf">());

static_assert(annotation::has<anno, "json", "req">());
static_assert(annotation::has<anno, "json", "required">());
static_assert(annotation::has<anno, "json", "omitempty">());
static_assert(!annotation::has<anno, "json", "">());
static_assert(!annotation::has<anno, "json", "unknown">());

static_assert(annotation::has<anno, "envconfig", "required">());
static_assert(annotation::has<anno, "envconfig", "name">());
static_assert(annotation::has<anno, "envconfig", "default_value">());
static_assert(!annotation::has<anno, "envconfig", "">());
static_assert(!annotation::has<anno, "envconfig", "default">());

static_assert(!annotation::has<anno, "unknown", "">());
static_assert(!annotation::has<anno, "unknown", "nothing">());

static_assert(annotation::get<anno, "json", 0>() == "json");
static_assert(annotation::get<anno, "json", 1>() == "req");
static_assert(annotation::get<anno, "json", 2>() == "required");
static_assert(annotation::get<anno, "json", 3>() == "omitempty");
static_assert(annotation::get<anno, "json", 4>().empty());
static_assert(annotation::get<anno, "json", 42>().empty());

static_assert(annotation::get<anno, "envconfig", 0>() == "envconfig");
static_assert(annotation::get<anno, "envconfig", 1>() == "required=true");
static_assert(annotation::get<anno, "envconfig", 2>() == "name=example_name");
static_assert(annotation::get<anno, "envconfig", 3>() == "default_value=");
static_assert(annotation::get<anno, "envconfig", 4>().empty());
static_assert(annotation::get<anno, "envconfig", 42>().empty());

static_assert(annotation::get<anno, "unknown", 42>().empty());

static_assert(annotation::get<anno, "json", "req">() == "req");
static_assert(annotation::get<anno, "json", "required">() == "required");
static_assert(annotation::get<anno, "json", "omitempty">() == "omitempty");
static_assert(annotation::get<anno, "json", "requ">().empty());

static_assert(annotation::get<anno, "envconfig", "required">() == "true");
static_assert(annotation::get<anno, "envconfig", "name">() == "example_name");
static_assert(annotation::get<anno, "envconfig", "default_value">() == "default_value");

} // namespace trezz::test
