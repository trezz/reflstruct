#pragma once

#include "reflstruct.h"

#include <_ctype.h>
#include <cstdlib>
#include <optional>
#include <string>
#include <type_traits>

namespace trezz::envconfig {

namespace detail {

template<typename T>
std::optional<std::string> process(std::string_view value, T& dest)
{
    using D = std::decay_t<decltype(dest)>;

    if constexpr (std::is_same_v<D, std::string>) {
        dest = std::string(value);
    } else {
        static_assert(!std::is_same_v<D, D>, "unsupported value type");
    }

    return std::nullopt;
}

} // namespace detail

template<typename T>
requires std::is_base_of_v<base_reflstruct, T> std::optional<std::string> process(T& dest)
{
    std::optional<std::string> err{ std::nullopt };
    dest.each([&](auto& member) {
        if (err.has_value()) {
            return;
        }

        using M = std::decay_t<decltype(member)>;

        if constexpr (annotation::has<M::annotation, "envconfig", "ignore">()) {
            return;
        }

        constexpr auto name = annotation::get<M::annotation, "envconfig", "name", M::name>();

        std::array<char, name.size() + 1> upper_name{};
        for (size_t i = 0; i < name.size(); ++i) {
            upper_name[i] = toupper(name[i]);
        }

        const char* value = std::getenv(upper_name.data());
        if (value == nullptr) {
            if constexpr (annotation::has<M::annotation, "envconfig", "required">()) {
                err = std::string("required '") + std::string(upper_name.data()) + "' not found";
            } else if constexpr (annotation::has<M::annotation, "envconfig", "default">()) {
                err = detail::process(annotation::get<M::annotation, "envconfig", "default">(),
                                      member.value);
            }
            return;
        }

        err = detail::process(value, member.value);
    });

    return err;
}

} // namespace trezz::envconfig
