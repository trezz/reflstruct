#pragma once

#include "envconfig.h"
#include "reflstruct.h"

namespace trezz::envconfig::test {

static_assert(envconfig::is_invalid_annotation<"">() == 0);
static_assert(envconfig::is_invalid_annotation<"json:required">() == 0);
static_assert(envconfig::is_invalid_annotation<"envconfig:required">() == 0);
static_assert(envconfig::is_invalid_annotation<"envconfig:required,ignore">() == 0);
static_assert(envconfig::is_invalid_annotation<"envconfig:default">() == 1);
static_assert(envconfig::is_invalid_annotation<"envconfig:ignore,name=MYNAME">() == 0);
static_assert(envconfig::is_invalid_annotation<"envconfig:required,unknown,ignore">() == 2);

} // namespace trezz::envconfig::test
