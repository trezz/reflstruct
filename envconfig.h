#pragma once

#include "reflstruct.h"

#include <cstdlib>
#include <exception>
#include <functional>
#include <string>
#include <type_traits>

namespace trezz::envconfig {

struct exception : public std::exception
{
    explicit exception(std::string message)
      : _message{ std::move(message) }
    {
    }

    const char* what() const noexcept override { return _message.data(); }

private:
    std::string _message{};
};

namespace detail {

template<typename T>
constexpr void process(std::string_view value, T& dest)
{
    using D = std::decay_t<decltype(dest)>;

    if constexpr (std::is_same_v<D, std::string>) {
        dest = std::string(value);
    } else if constexpr (std::is_same_v<D, std::string_view>) {
        dest = value;
    } else {
        static_assert(!std::is_same_v<D, D>, "unsupported value type");
    }
}

template<trezz::detail::string_literal Annotation, size_t N>
constexpr size_t is_invalid_annotation()
{
    if constexpr (N == 0) {
        return 0;
    } else {
        constexpr auto element = annotation::get<Annotation, "envconfig", N>();
        if constexpr (element == "ignore" || element == "required" ||
                      element.starts_with("name=")) {
            return is_invalid_annotation<Annotation, N - 1>();
        } else {
            return N;
        }
    }
}

template<typename T, typename Fn>
requires std::is_base_of_v<base_reflstruct, T> && std::is_invocable_r_v<char*, Fn, const char*>
void process(T& dest, const Fn& env_getter)
{
    dest.each([&](auto& member) {
        using M = std::decay_t<decltype(member)>;

        constexpr auto invalid_element_pos = is_invalid_annotation<M::annotation, "envconfig">();
        static_assert(invalid_element_pos == 0, "TODO: show the index");

        if constexpr (annotation::has<M::annotation, "envconfig", "ignore">()) {
            return;
        }

        constexpr auto name = annotation::get<M::annotation, "envconfig", "name", M::name>();

        std::array<char, name.size() + 1> upper_name{};
        for (size_t i = 0; i < name.size(); ++i) {
            upper_name[i] = toupper(name[i]);
        }

        const char* value = env_getter(upper_name.data());
        if (value == nullptr) {
            if constexpr (annotation::has<M::annotation, "envconfig", "required">()) {
                throw envconfig::exception("required '" + std::string(upper_name.data()) +
                                           "' not found");
            } else {
                return;
            }
        }

        detail::process(value, member.value);
    });
}

} // namespace detail

// Return the index of the first element in the annotation configuration of envconfig that is
// invalid, or 0 if the configuration is valid.
template<trezz::detail::string_literal Annotation>
constexpr size_t is_invalid_annotation()
{
    constexpr auto n = annotation::nb_configuration_elements<Annotation, "envconfig">();
    if constexpr (n == 0) {
        return 0;
    } else {
        return detail::is_invalid_annotation<Annotation, n>();
    }
}

// Fill the given reflstruct with the values found in the environment.
// An exception of type trezz::envconfig::exception is thrown on error.
template<typename T>
requires std::is_base_of_v<base_reflstruct, T>
void process(T& dest)
{
    detail::process(dest, std::getenv);
}

} // namespace trezz::envconfig
