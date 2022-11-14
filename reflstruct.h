#pragma once

#include <__tuple>
#include <string_view>
#include <tuple>

namespace trezz {

namespace detail {

// Store a character string as a constexpr value that can be passed as non-type template parameter.
template<size_t size>
struct string_literal
{
    constexpr string_literal(const char (&s)[size]) { std::copy_n(s, size, data); }
    char data[size];
};

} // namespace detail

// A struct member with reflected name, type and annotation.
template<typename T, detail::string_literal Name, detail::string_literal Annotation = "">
struct reflmember
{
    // Member name as a string literal.
    static constexpr auto literal_name{ Name };

    // Member name.
    static constexpr std::string_view name{ Name.data };

    // Member type.
    using value_type = T;

    // Member annotation as a string literal.
    static constexpr auto annotation{ Annotation };

    constexpr reflmember() = default;
    explicit constexpr reflmember(T&& v)
      : value{ std::forward<T>(v) }
    {
    }

    // Member value.
    T value{};
};

// Parent type of all reflstruct.
struct base_reflstruct
{};

template<typename... Ts>
struct reflstruct : base_reflstruct
{
    // Number of members of the struct.
    static constexpr size_t nb_members{ sizeof...(Ts) };

    constexpr reflstruct() = default;

    // Construct a reflstruct with reflmembers as arguments.
    explicit constexpr reflstruct(Ts&&... members)
      : _members{ std::forward<Ts>(members)... }
    {
    }

    // Return the value of a member selected by its name.
    template<detail::string_literal Name>
    constexpr auto& get()
    {
        return member<Name>().value;
    }

    // Return the value of a member selected by its name.
    template<detail::string_literal Name>
    constexpr const auto& get() const
    {
        return member<Name>().value;
    }

    // Return true if the struct contains a member with the given name, false otherwise.
    template<size_t N = 0>
    static constexpr bool contains(std::string_view member_name)
    {
        if constexpr (N < nb_members) {
            using Member = std::tuple_element_t<N, decltype(_members)>;
            if (Member::name == member_name) {
                return true;
            } else {
                return contains<N + 1>(member_name);
            }
        } else {
            return false;
        }
    }

    // Return the member with the given name.
    template<detail::string_literal Name, size_t N = 0>
    constexpr auto& member()
    {
        static_assert(N < nb_members, "invalid member name");
        using Member = std::tuple_element_t<N, decltype(_members)>;
        if constexpr (Member::name == std::string_view{ Name.data }) {
            return std::get<N>(_members);
        } else {
            return member<Name, N + 1>();
        }
    }

    // Return the member with the given name.
    template<detail::string_literal Name, size_t N = 0>
    constexpr const auto& member() const
    {
        static_assert(N < nb_members, "invalid member name");
        using Member = std::tuple_element_t<N, decltype(_members)>;
        if constexpr (Member::name == std::string_view{ Name.data }) {
            return std::get<N>(_members);
        } else {
            return member<Name, N + 1>();
        }
    }

    // Call the given function on each members of the struct, with the member given as input
    // argument to the function.
    template<typename Fn>
    constexpr void each(const Fn& f)
    {
        _each(f, std::make_index_sequence<nb_members>{});
    }

    // Call the given function on each members of the struct, with the member given as input
    // argument to the function.
    template<typename Fn>
    constexpr void each(const Fn& f) const
    {
        _each(f, std::make_index_sequence<nb_members>{});
    }

private:
    template<typename Fn, std::size_t... Is>
    void _each(const Fn& f, std::index_sequence<Is...>) const
    {
        (f(std::get<Is>(_members)), ...);
    }

    template<typename Fn, std::size_t... Is>
    void _each(const Fn& f, std::index_sequence<Is...>)
    {
        (f(std::get<Is>(_members)), ...);
    }

    std::tuple<Ts...> _members{};
};

namespace annotation {

/*

reflmember annotations are configurations parsed and interpreted at compile-time that can be used to
configure how reflection-based library use reflmembers.

It is inspired from the Golang struct tags.

The syntax is:
  Annotation ::= Configurations
  Configurations ::=
      Configuration
    | Configuration Spaces Configurations
  Configuration ::= Value ":" Elements
  Elements ::=
      Element
    | Element "," Elements
  Element ::=
      Value
    | Value "=" Value
  Spaces ::=
      Space
    | Space Spaces
  Space ::= " " | "\n" | "\t" | "\r"
  Value ::= <any characters except spaces, ':', '=' and ','>

*/

namespace detail {

template<trezz::detail::string_literal Annotation, trezz::detail::string_literal ConfigurationName>
inline constexpr std::string_view configuration_elements()
{
    constexpr std::string_view annotation{ Annotation.data };
    constexpr std::string_view conf_name{ ConfigurationName.data };

    size_t n = 0; // Cursor in the current configuration.
    bool skipConf = false;

    const auto isSpace = [](char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; };

    for (size_t i = 0; i < annotation.size(); ++i) {
        const auto c = annotation[i];

        if (skipConf) {
            if (isSpace(c)) {
                skipConf = false;
            } else {
                continue;
            }
        }

        // Skip spaces.
        if (isSpace(c)) {
            n = 0;
            continue;
        }

        // Beginning of a conf.
        if (n < conf_name.size() && c == conf_name[n]) {
            n++;
            continue;
        }

        // Config name was fully matched.
        if (c == ':' && n == conf_name.size()) {
            const size_t start = i + 1;
            size_t j = start;
            for (; j < annotation.size(); ++j) {
                if (isSpace(annotation[j])) {
                    break;
                }
            }
            const auto size = j - start;
            return { &annotation[start], size };
        }

        n = 0;
        skipConf = true;
    }

    return {};
}

} // namespace detail

// Return the number of elements of the given configuration in the given annotation.
template<trezz::detail::string_literal Annotation, trezz::detail::string_literal ConfigurationName>
inline constexpr size_t nb_configuration_elements()
{
    constexpr auto elements = detail::configuration_elements<Annotation, ConfigurationName>();
    if constexpr (elements.empty()) {
        return 0;
    } else {
        return std::count(elements.begin(), elements.end(), ',') + 1;
    }
}

// Return the element of the given configuration at the given position, or the given default value
// if the element is not found.
template<trezz::detail::string_literal Annotation,
         trezz::detail::string_literal ConfigurationName,
         size_t ElementPosition,
         trezz::detail::string_literal DefaultValue = "">
inline constexpr std::string_view get()
{
    constexpr std::string_view elements =
        detail::configuration_elements<Annotation, ConfigurationName>();

    if (elements.empty()) {
        return { DefaultValue.data };
    }

    if (ElementPosition == 0) {
        return std::string_view{ ConfigurationName.data };
    }

    size_t start = 0;
    size_t size = 0;
    size_t n = 1;

    for (char c : elements) {
        if (c != ',') {
            size++;
            continue;
        }

        if (n == ElementPosition) {
            break;
        }

        n++;
        start += size + 1;
        size = 0;
    }

    if (n == ElementPosition) {
        return { &elements[start], size };
    }

    return { DefaultValue.data };
}

// Return the value of the given element of the given configuration, or the given default value if
// the element is not found.
// If the element is found but doesn't have an associated value, the element name is returned.
template<trezz::detail::string_literal Annotation,
         trezz::detail::string_literal ConfigurationName,
         trezz::detail::string_literal ElementName,
         trezz::detail::string_literal DefaultValue = "",
         size_t N = 0>
inline constexpr std::string_view get()
{
    constexpr std::string_view element_name{ ElementName.data };
    constexpr size_t max = nb_configuration_elements<Annotation, ConfigurationName>();

    if constexpr (N > max) {
        return std::string_view{ DefaultValue.data };
    } else {
        constexpr std::string_view nth_element = get<Annotation, ConfigurationName, N>();
        if constexpr (nth_element.starts_with(element_name)) {
            if constexpr (nth_element.size() == element_name.size()) {
                return element_name;
            } else if constexpr (nth_element[element_name.size()] == '=') {
                constexpr std::string_view element_value{ &nth_element[element_name.size() + 1],
                                                          nth_element.size() - element_name.size() -
                                                              1 };
                if constexpr (element_value.empty()) {
                    return element_name;
                } else {
                    return element_value;
                }
            } else {
                return get<Annotation, ConfigurationName, ElementName, DefaultValue, N + 1>();
            }
        } else {
            return get<Annotation, ConfigurationName, ElementName, DefaultValue, N + 1>();
        }
    }
}

// Return true if the annotation contains the given configuration, false otherwise.
template<trezz::detail::string_literal Annotation, trezz::detail::string_literal ConfigurationName>
inline constexpr bool has()
{
    return !get<Annotation, ConfigurationName, 0>().empty();
}

// Return true if the annotation contains the given element of the given configuration, false
// otherwise.
template<trezz::detail::string_literal Annotation,
         trezz::detail::string_literal ConfigurationName,
         trezz::detail::string_literal ElementName>
inline constexpr bool has()
{
    return !get<Annotation, ConfigurationName, ElementName>().empty();
}

} // namespace annotation

} // namespace trezz
