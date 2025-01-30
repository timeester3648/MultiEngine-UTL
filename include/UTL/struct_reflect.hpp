// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::struct_reflect
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_struct_reflect.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_STRUCT_REFLECT)
#ifndef UTLHEADERGUARD_STRUCT_REFLECT
#define UTLHEADERGUARD_STRUCT_REFLECT

// _______________________ INCLUDES _______________________

#include <array>       // array<>
#include <cstddef>     // size_t
#include <string_view> // string_view
#include <tuple>       // tuple<>, tuple_size<>, apply<>(), get<>()
#include <type_traits> // add_lvalue_reference_t<>, add_const_t<>, remove_reference_t<>, decay_t<>
#include <utility>     // forward<>(), pair<>

// ____________________ DEVELOPER DOCS ____________________

// Reflection mechanism is based entirely around the map macro and a single struct with partial specialization for the
// reflected enum. Map macro itself is quire non-trivial, but completely standard, a good explanation of how it works
// can be found here: [https://github.com/swansontec/map-macro].
//
// Once we have a map macro all reflection is a matter of simply mapping __VA_ARGS__ into various
// arrays and tuples, which allows us to work with structures in a generic tuple-like way.
//
// Partial specialization allows for a pretty concise implementation and provides nice error messages due to
// static_assert on incorrect template arguments.
//
// An alternative frequently used way to do struct reflection is through generated code with structured binding &
// hundreds of overloads. This has a benefit of producing nicer error messages on 'for_each()' however the
// resulting implementation is downright abnorrent.

// ____________________ IMPLEMENTATION ____________________

namespace utl::struct_reflect {

// =================
// --- Map macro ---
// =================

#define utl_srfl_eval_0(...) __VA_ARGS__
#define utl_srfl_eval_1(...) utl_srfl_eval_0(utl_srfl_eval_0(utl_srfl_eval_0(__VA_ARGS__)))
#define utl_srfl_eval_2(...) utl_srfl_eval_1(utl_srfl_eval_1(utl_srfl_eval_1(__VA_ARGS__)))
#define utl_srfl_eval_3(...) utl_srfl_eval_2(utl_srfl_eval_2(utl_srfl_eval_2(__VA_ARGS__)))
#define utl_srfl_eval_4(...) utl_srfl_eval_3(utl_srfl_eval_3(utl_srfl_eval_3(__VA_ARGS__)))
#define utl_srfl_eval(...) utl_srfl_eval_4(utl_srfl_eval_4(utl_srfl_eval_4(__VA_ARGS__)))

#define utl_srfl_map_end(...)
#define utl_srfl_map_out
#define utl_srfl_map_comma ,

#define utl_srfl_map_get_end_2() 0, utl_srfl_map_end
#define utl_srfl_map_get_end_1(...) utl_srfl_map_get_end_2
#define utl_srfl_map_get_end(...) utl_srfl_map_get_end_1
#define utl_srfl_map_next_0(test, next, ...) next utl_srfl_map_out
#define utl_srfl_map_next_1(test, next) utl_srfl_map_next_0(test, next, 0)
#define utl_srfl_map_next(test, next) utl_srfl_map_next_1(utl_srfl_map_get_end test, next)

#define utl_srfl_map_0(f, x, peek, ...) f(x) utl_srfl_map_next(peek, utl_srfl_map_1)(f, peek, __VA_ARGS__)
#define utl_srfl_map_1(f, x, peek, ...) f(x) utl_srfl_map_next(peek, utl_srfl_map_0)(f, peek, __VA_ARGS__)

#define utl_srfl_map_list_next_1(test, next) utl_srfl_map_next_0(test, utl_srfl_map_comma next, 0)
#define utl_srfl_map_list_next(test, next) utl_srfl_map_list_next_1(utl_srfl_map_get_end test, next)

#define utl_srfl_map_list_0(f, x, peek, ...)                                                                           \
    f(x) utl_srfl_map_list_next(peek, utl_srfl_map_list_1)(f, peek, __VA_ARGS__)
#define utl_srfl_map_list_1(f, x, peek, ...)                                                                           \
    f(x) utl_srfl_map_list_next(peek, utl_srfl_map_list_0)(f, peek, __VA_ARGS__)

// Applies the function macro 'f' to all '__VA_ARGS__'
#define utl_srfl_map(f, ...) utl_srfl_eval(utl_srfl_map_1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

// Applies the function macro 'f' to to all '__VA_ARGS__' and inserts commas between the results
#define utl_srfl_map_list(f, ...) utl_srfl_eval(utl_srfl_map_list_1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

// Note: 'srfl' is short for 'struct_reflect'

// =========================
// --- Struct reflection ---
// =========================

// --- Implementation ---
// ----------------------

template <class>
inline constexpr bool _always_false_v = false;

template <class E>
struct _meta {
    static_assert(_always_false_v<E>,
                  "Provided struct does not have a defined reflection. Use 'UTL_STRUCT_REFLECT' macro to define one.");
    // makes instantiation of this template a compile-time error
};

// clang-format off
#define utl_srfl_type_name(arg_) std::string_view
#define utl_srfl_type_cref(arg_) std::add_lvalue_reference_t<std::add_const_t<std::remove_reference_t<decltype(type::arg_)>>>
#define utl_srfl_type_ref(arg_) std::add_lvalue_reference_t<std::remove_reference_t<decltype(type::arg_)>>
#define utl_srfl_type_centry(arg_) std::pair<utl_srfl_type_name(arg_), utl_srfl_type_cref(arg_)>
#define utl_srfl_type_entry(arg_) std::pair<utl_srfl_type_name(arg_), utl_srfl_type_ref(arg_)>

#define utl_srfl_make_name(arg_) utl_srfl_type_name(arg_)(#arg_)
#define utl_srfl_make_cref(arg_) val.arg_
#define utl_srfl_make_ref(arg_) val.arg_
#define utl_srfl_make_centry(arg_) utl_srfl_type_centry(arg_){ utl_srfl_make_name(arg_), utl_srfl_make_cref(arg_) }
#define utl_srfl_make_entry(arg_) utl_srfl_type_entry(arg_){ utl_srfl_make_name(arg_), utl_srfl_make_ref(arg_) }

#define utl_srfl_call_unary_func(arg_) func(std::forward<S>(val).arg_);
#define utl_srfl_call_binary_func(arg_) func(std::forward<S1>(val_1).arg_, std::forward<S2>(val_2).arg_);
// clang-format on

// Note: 'c' prefix means 'const'

#define UTL_STRUCT_REFLECT(struct_name_, ...)                                                                          \
    template <>                                                                                                        \
    struct utl::struct_reflect::_meta<struct_name_> {                                                                  \
        using type = struct_name_;                                                                                     \
                                                                                                                       \
        using fields_cref_type  = std::tuple<utl_srfl_map_list(utl_srfl_type_cref, __VA_ARGS__)>;                      \
        using entries_cref_type = std::tuple<utl_srfl_map_list(utl_srfl_type_centry, __VA_ARGS__)>;                    \
        using fields_ref_type   = std::tuple<utl_srfl_map_list(utl_srfl_type_ref, __VA_ARGS__)>;                       \
        using entries_ref_type  = std::tuple<utl_srfl_map_list(utl_srfl_type_entry, __VA_ARGS__)>;                     \
                                                                                                                       \
        constexpr static std::string_view type_name = #struct_name_;                                                   \
                                                                                                                       \
        constexpr static auto names = std::array{utl_erfl_map_list(utl_erfl_make_name, __VA_ARGS__)};                  \
                                                                                                                       \
        constexpr static fields_cref_type const_field_view(const type& val) {                                          \
            return {utl_srfl_map_list(utl_srfl_make_cref, __VA_ARGS__)};                                               \
        }                                                                                                              \
        constexpr static fields_ref_type field_view(type& val) {                                                       \
            return {utl_srfl_map_list(utl_srfl_make_ref, __VA_ARGS__)};                                                \
        }                                                                                                              \
        constexpr static entries_cref_type const_entry_view(const type& val) {                                         \
            return {utl_srfl_map_list(utl_srfl_make_centry, __VA_ARGS__)};                                             \
        }                                                                                                              \
        constexpr static entries_ref_type entry_view(type& val) {                                                      \
            return {utl_srfl_map_list(utl_srfl_make_entry, __VA_ARGS__)};                                              \
        }                                                                                                              \
                                                                                                                       \
        constexpr static std::size_t size = std::tuple_size_v<_meta::fields_cref_type>;                                \
                                                                                                                       \
        template <std::size_t I>                                                                                       \
        constexpr static auto get(const type& val) {                                                                   \
            return std::get<I>(_meta::const_field_view(val));                                                          \
        }                                                                                                              \
                                                                                                                       \
        template <std::size_t I>                                                                                       \
        constexpr static auto get(type& val) {                                                                         \
            return std::get<I>(_meta::field_view(val));                                                                \
        }                                                                                                              \
                                                                                                                       \
        template <class S, class Func>                                                                                 \
        constexpr static void for_each(S&& val, Func&& func) {                                                         \
            utl_srfl_map(utl_srfl_call_unary_func, __VA_ARGS__)                                                        \
        }                                                                                                              \
                                                                                                                       \
        template <class S1, class S2, class Func>                                                                      \
        constexpr static void for_each(S1&& val_1, S2&& val_2, Func&& func) {                                          \
            utl_srfl_map(utl_srfl_call_binary_func, __VA_ARGS__)                                                       \
        }                                                                                                              \
    }

// --- Public API ---
// ------------------

template <class S>
constexpr auto type_name = _meta<S>::type_name;

template <class S>
constexpr auto names = _meta<S>::names;

template <class S>
constexpr auto field_view(const S& value) {
    return _meta<S>::const_field_view(value);
}

template <class S>
constexpr auto field_view(S& value) {
    return _meta<S>::field_view(value);
}

template <class S>
constexpr auto entry_view(const S& value) {
    return _meta<S>::const_entry_view(value);
}

template <class S>
constexpr auto entry_view(S& value) {
    return _meta<S>::entry_view(value);
}

template <class S>
constexpr auto size = _meta<S>::size;

template <std::size_t I, class S>
constexpr auto get(S&& value) {
    using struct_type = typename std::decay_t<S>;
    return _meta<struct_type>::template get<I>(value);
}

template <class S, class Func>
constexpr auto for_each(S&& value, Func&& func) {
    using struct_type = typename std::decay_t<S>;
    return _meta<struct_type>::template for_each(value, func);
}

template <class S1, class S2, class Func>
constexpr auto for_each(S1&& value_1, S2&& value_2, Func&& func) {
    using struct_type_1 = typename std::decay_t<S1>;
    using struct_type_2 = typename std::decay_t<S2>;
    static_assert(std::is_same_v<struct_type_1, struct_type_2>,
                  "Called 'struct_reflect::for_each(s1, s2, func)' with incompatible argument types.");
    return _meta<struct_type_1>::template for_each(value_1, value_2, func);
}

// --- Misc utils ---
// ------------------

// Struct reflection provides its own 'for_each()' with no tuple magic, this function is useful
// in case user want to operate on tuples rather than structs using similar API, sort of a "bonus utility"
// that simply doesn't have any better module to be a part of
template <class T, class Func>
void tuple_for_each(T&& tuple, Func&& func) {
    std::apply([&func](auto&&... args) { (func(std::forward<decltype(args)>(args)), ...); }, std::forward<T>(tuple));
}

// For a pair of tuple 'std::apply' trick doesn't cut it, gotta do the standard thing
// with recursion over the index sequence. This looks kinda horrible
template <class T1, class T2, class Func, std::size_t... Idx>
static void _tuple_for_each_impl(T1&& tuple_1, T2&& tuple_2, Func&& func, std::index_sequence<Idx...>) {
    (func(std::get<Idx>(std::forward<T1>(tuple_1)), std::get<Idx>(std::forward<T2>(tuple_2))), ...);
    // fold expression '( f(args), ... )' invokes 'f(args)' for all indeces in the index sequence
}

template <template <class...> class T1, template <class...> class T2, class Func, class... Args>
static void _tuple_for_each_fwd(T1<Args...>&& tuple_1, T2<Args...>&& tuple_2, Func&& func) {
    _tuple_for_each_impl(std::forward<T1<Args...>>(tuple_1), std::forward<T2<Args...>>(tuple_2), std::forward<Func>(func),
                         std::index_sequence_for<Args...>{});
    // forward argument while deducing properly sized index sequence
}

template <class T1, class T2, class Func>
void tuple_for_each(T1&& tuple_1, T2&& tuple_2, Func&& func) {
    _tuple_for_each_fwd(std::forward<T1>(tuple_1), std::forward<T2>(tuple_2), std::forward<Func>(func));
}

} // namespace utl::struct_reflect

#endif
#endif // module utl::struct_reflect
