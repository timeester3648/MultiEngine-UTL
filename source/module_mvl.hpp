// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/prototyping_utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::mvl
// Documentation: https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_mvl.md
// Source repo:   https://github.com/DmitriBogdanov/prototyping_utils
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_MVL)
#ifndef UTLHEADERGUARD_MVL
#define UTLHEADERGUARD_MVL

// _______________________ INCLUDES _______________________

#include <algorithm>        // swap(), find(), count(), is_sorted(), min_element(),
                            // max_element(), sort(), stable_sort(), min(), max(), remove_if(), copy()
#include <cmath>            // isfinite()
#include <cstddef>          // size_t, ptrdiff_t, nullptr_t
#include <functional>       // reference_wrapper<>, multiplies<>
#include <initializer_list> // initializer_list<>
#include <iomanip>          // setw()
#include <ios>              // right(), boolalpha(), ios::boolalpha
#include <iterator>         // random_access_iterator_tag, reverse_iterator<>
#include <memory>           // unique_ptr<>
#include <numeric>          // accumulate()
#include <ostream>          // ostream
#include <sstream>          // ostringstream
#include <stdexcept>        // out_of_range, invalid_argument
#include <string>           // string
#include <type_traits>      // conditional_t<>, enable_if_t<>, void_t<>, true_type, false_type, remove_reference_t<>
#include <utility>          // move()
#include <vector>           // vector<>

// ____________________ DEVELOPER DOCS ____________________

// This module tries to implement an "unreasonably flexible yet convenient" template for vectors and matrices,
// in order to do so we have to rely heavily on either conditional compilation or automatic code generation
// (or both). There are multiple seemingly viable approaches to it, yet the most of them proved to be either
// unsuitable or unreasonable from the implementation standpoint. Below is a little rundown of implementations
// that were attempted and used/discared for various reasons.
//
// Currently matrix/vector/view/etc code reuse is implemented though a whole buch of conditional compilation with
// SFINAE, abuse of conditional inheritance, conditional types and constexpr if's. While not perfect, this is
// the best working approach so far. Below is a list of approaches that has been considered & tried:
//
// 1) No code reuse, all classes implemented manually - unrealistically cumbersome, huge code duplication
//
//    => [-] UNSUITABLE APPROACH
//
// 2) Regular OOP with virtual classes - having vtables in lightweigh containers is highly undesirable, we can
//    avoid this using CRTP however we run into issues with multiple inheritance (see below)
//
//    => [-] UNSUITABLE APPROACH
//
// 3) CRTP - we unavoidably run into multiple inheritance issues (diamond problem) with concepts like:
//       [mutable-matrix-like] : [const-matrix-like] & [mutable-iterable] where both parents derive from
//       [const-iterable]
//
//    While those issues are resolvable, their handling becomes more and more difficult as parent classes grow larger.
//    A usual solution for this would be virtual inheritance, however it prevents us from calling derived methods inside
//    the base class due to "static downcast via virtual inheritance" rule, which makes 'static_cast<Derived*>(this)'
//    illegal. We can resolve all name collisions manually, however it leads to a code that is absolutely horrific and
//    easy to mess up.
//
//    There is also an issue of extreme boilerplate, since we want to propagate '_types<>' wrapper and the 'Final'
//    return type to all base classes we end up with extremely unwieldy template arguments that make compile errors and
//    refactoring a challenge.
//
//    => [~] SUITABLE, BUT VERY PROBLEMATIC APPROACH
//
// 4) Template-based mixins without CRTP - these allow to add functionality on top of a class, essentially it works
//    like "inverted" CRTP from the code logic POV. While being almost a perfect solution, it doesn't allow us to
//    return objects of derived type from base class methods, which is a requirement for the intended matrix API.
//
//    => [-] UNSUITABLE APPROACH
//
// 5) Macro-based mixins - insert functionality directly in a class, a simple way of doing exactly what we want.
//    "Mixed-in" methods are simply functions implemented in terms of certain "required" methods.
//    Doesn't lead to huge template chains and doesn't have any inheritance at all. The main downside is being
//    preprocessor based, which exposes some identifiers to a global namespace and makes all "connection points"
//    in logic dependant on correct naming inside macros, rather than actual semantical symbols.
//
//    Difficult to work with due to being ignored by the language server, gets increasingly cumbersome as the number of
//    classes grows and still has significant code duplication (vector/matrix * dense/strided/sparse *
//    * container/view/const_view = 18 manually created definitions).
//
//    => [+-] SUITABLE, BUT HEAVILY FLAWED APPROACH
//
// 6) A single class with all pars of API correctly enabled/disabled through SFINAE. Closes thing to a sensible
//    approach with minimal (basically none) code duplication. However it has a number of non-trivial quirks
//    when it comes to conditionally compiling member variables, types & functions (every group has a quirk of
//    its own, functions are the least problematic). Some of such quirks limit possible conditional API (member
//    types suffer from this) or, for example, prevent use of initialization lists on conditionally compiled
//    constructors (big sad).
//
//    A less obvious downside compared to macros, is that it doesn't get handles as well by the language server
//    autocomplete - SFINAE-disabled methods still show up as autocomplete suggestions, even if they get marked
//    as missing immediately upon writing the whole name.
//
//    => [+] DIFFICULT, BUT WORKABLE, BEST APPROACH SO FAR
//
// NOTES ON ALGEBRAIC OPERATIONS:
//
// While it is certainly possible to implement all the necessary operators, especially if doing so in a "dumb"
// way that takes everything by const reference and returns a copy, doing things in a "smart" way with perfect
// forwarding and proper r-value reuse (which would reduce the number of copies in a chain of N operator from
// N to 1) is absolutely horrific due to the necessity to SFINAE-restrict all implemenations. Doing this part
// of the library would be both much cleaner and easier in C++20 (and C++23) with following features being the key:
//    - C++20 concepts        - makes restricting operations much easier and cleaner than with SFINAE
//    - C++23 Deducing 'this' - allows operators to perfectly forward '*this' which allows them to be declared as
//                              member functions and not as free-standing ones, this considerably reduces the
//                              room to mess things up by implementing improper operator restrictions.
// An example and some key notes on how to properly handle perfect forwarding and operators is saved in one of
// the backup snippets, but in the nutshell there are following points:
//    1. All unary and binary operators ('-', '+', '*' and etc.) can benefit from reusing r-values and moving
//       one of the arguments into the result rather than copy. This can only happen to CONTAINER args that
//       correspond to the return type.
//    2. Binary operators have following return types for different tensors:
//       -  dense +  dense =>  dense      (complexity O(N^2))
//       -  dense + sparse =>  dense      (complexity O(N)  )
//       - sparse +  dense =>  dense      (complexity O(N)  )
//       - sparse + sparse => sparse      (complexity O(N)  )
//       return types inherit option arguments from the lhs and always have 'ownership == CONTAINER',
//       while argument can be anything, including views. To implement the list above efficiently there
//       is no other way but to make 4 separate implementation, tailored for each level of sparsity.
//       Working snippet for 'sparse + sparse' and pseudocode for others can be found in backups.
//    3. All binary operators can be written in a generic form like 'apply_binary_operator(A, B, op)'
//       and specialized with operator functors froms std <functional>. All we need for individual
//       operators is to figure out correct boilerplate with perfrect forwarding.
//
// NOTES ON SPAN:
// 
// We can easily implement Matlab-like API to take matrix blocks like this:
//    matrix(Span{0, 10}, 4)
// which would be equivalent to
//    matrix.block(0, 4, 10, 1)
// we only need a thin 'Span' POD with 2 members and a few overloads for 'operator()' that redirect span
// to 'this->block(...)'
//
// NOTES ON NAMING:
//
// Macro naming is a bit of a mess as of now.

// ____________________ IMPLEMENTATION ____________________

namespace utl::mvl {

// =======================
// --- Utility Classes ---
// =======================

// Wrapper that allows passing standard set of member types down the CRTP chain
// (types have to be passed through template args one way or another. Trying to access
// 'derived_type::value_type' from the CRTP base class will fail, since in order for it to be available
// the derived class has to be "implemented" which can only happen after the base class is "implemented",
// which requires the type => a logical loop. So we wrap all the types in a class a pass them down
// the chain a "pack with everything" type of deal. This pack then gets "unwrapped" with
// '_utl_storage_define_types' macro in every class => we have all the member types defined).
//
// (!) Since CRTP approach has been deprecated in favor of mixins, the original reasoning is no longer
// accurate, however it is this a useful class whenever a pack of types have to be passed through a
// template (like, for example, in iterator implementation)
template <class T>
struct _types {
    using value_type      = T;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = T&;
    using const_reference = const T&;
    using pointer         = T*;
    using const_pointer   = const T*;
};

// A minimal equivalent of the once proposed 'std::observer_ptr<>'.
// Used in views to allow observer pointers with the same interface as std::unique_ptr,
// which means a generic '.data()` can be implemented without having to make a separate version for views and containers
template <class T>
class _observer_ptr {
public:
    using element_type = T;

private:
    element_type* _data = nullptr;

public:
    // - Constructors -
    constexpr _observer_ptr() noexcept = default;
    constexpr _observer_ptr(std::nullptr_t) noexcept {}
    explicit _observer_ptr(element_type* ptr) : _data(ptr) {}

    _observer_ptr& operator=(element_type* ptr) {
        this->_data = ptr;
        return *this;
    }

    // - Interface -
    [[nodiscard]] constexpr element_type* get() const noexcept { return _data; }
};

// =============
// --- Enums ---
// =============

// Parameter enums
// They specify compiler tensor API
enum class Dimension { VECTOR, MATRIX };
enum class Type { DENSE, STRIDED, SPARSE };
enum class Ownership { CONTAINER, VIEW, CONST_VIEW };

// Config enums
// They specify conditional logic for a tensor
// Their combination + parameter enums fully defines a GenericTensor
enum class Checking { NONE, BOUNDS };
enum class Layout { /* 1D */ FLAT, /* 2D */ RC, CR, /* Other */ SPARSE };

// Overload tags (dummy types used to create "overloads" of .for_each(), which changes the way compiler handles
// name based lookup, preventing shadowing of base class methods)
// TODO: Most likely unnecessary after switching away from CRTP, try removing
enum class _for_each_tag { DUMMY };
enum class _for_each_idx_tag { DUMMY };
enum class _for_each_ij_tag { DUMMY };

// =================
// --- Iterators ---
// =================

// Iterator template for 1D iterator over a tensor.
// Reduces code duplication for const & non-const variants by bundling some conditional logic.
// Uses 'operator[]' of 'ParentPointer' for all logic, thus allowing arbitrary tensors that
// don't have to be contiguous or even ordered in memory.
template <class ParentPointer, class Types, bool is_const_iter = false>
class _flat_iterator {
    using parent_pointer = ParentPointer;

public:
    // Iterator reqs: [General]
    // Contains member types      =>
    using iterator_category = std::random_access_iterator_tag;
    using difference_type   = typename Types::difference_type;
    using value_type        = typename Types::value_type;
    using pointer = typename std::conditional_t<is_const_iter, typename Types::const_pointer, typename Types::pointer>;
    using reference =
        typename std::conditional_t<is_const_iter, typename Types::const_reference, typename Types::reference>;

    // Iterator reqs: [General]
    // Constructible
    _flat_iterator(parent_pointer parent, difference_type idx) : _parent(parent), _idx(idx) {}
    // Copy-constructible => by default
    // Copy-assignable    => by default
    // Destructible       => by default
    // Swappable
    friend void swap(_flat_iterator& lhs, _flat_iterator& rhs) { std::swap(lhs._idx, rhs._idx); }

    // Iterator reqs: [General]
    // Can be incremented (prefix & postfix)
    _flat_iterator& operator++() {
        ++_idx;
        return *this;
    } // prefix
    _flat_iterator operator++(int) {
        _flat_iterator temp = *this;
        ++_idx;
        return temp;
    } // postfix

    // Iterator reqs: [Input iterator]
    // Supports equality/inequality comparisons
    friend bool operator==(const _flat_iterator& it1, const _flat_iterator& it2) { return it1._idx == it2._idx; };
    friend bool operator!=(const _flat_iterator& it1, const _flat_iterator& it2) { return it1._idx != it2._idx; };
    // Can be dereferenced as an rvalue
    reference   operator*() const { return _parent->operator[](_idx); }
    pointer     operator->() const { return &_parent->operator[](_idx); }

    // Iterator reqs: [Output iterator]
    // Can be dereferenced as an lvalue (only for mutable iterator types) => not needed

    // Iterator reqs: [Forward iterator]
    // Default-constructible
    _flat_iterator() : _parent(nullptr), _idx(0) {}
    // "Multi-pass" - dereferencing & incrementing does not affects dereferenceability => satisfied

    // Iterator reqs: [Bidirectional iterator]
    // Can be decremented
    _flat_iterator& operator--() {
        --_idx;
        return *this;
    } // prefix
    _flat_iterator operator--(int) {
        _flat_iterator temp = *this;
        --_idx;
        return temp;
    } // postfix

    // Iterator reqs: [Random Access iterator]
    // See: https://en.cppreference.com/w/cpp/named_req/RandomAccessIterator
    // Supports arithmetic operators: it + n, n + it, it - n, it1 - it2
    friend _flat_iterator operator+(const _flat_iterator& it, difference_type diff) {
        return _flat_iterator(it._parent, it._idx + diff);
    }
    friend _flat_iterator operator+(difference_type diff, const _flat_iterator& it) {
        return _flat_iterator(it._parent, it._idx + diff);
    }
    friend _flat_iterator operator-(const _flat_iterator& it, difference_type diff) {
        return _flat_iterator(it._parent, it._idx - diff);
    }
    friend difference_type operator-(const _flat_iterator& it1, const _flat_iterator& it2) {
        return it1._idx - it2._idx;
    }
    // Supports inequality comparisons (<, >, <= and >=) between iterators
    friend bool     operator<(const _flat_iterator& it1, const _flat_iterator& it2) { return it1._idx < it2._idx; }
    friend bool     operator>(const _flat_iterator& it1, const _flat_iterator& it2) { return it1._idx > it2._idx; }
    friend bool     operator<=(const _flat_iterator& it1, const _flat_iterator& it2) { return it1._idx <= it2._idx; }
    friend bool     operator>=(const _flat_iterator& it1, const _flat_iterator& it2) { return it1._idx >= it2._idx; }
    // Standard assumption: Both iterators are from the same container => no need to compare '_parent'
    // Supports compound assignment operations += and -=
    _flat_iterator& operator+=(difference_type diff) {
        _idx += diff;
        return *this;
    }
    _flat_iterator& operator-=(difference_type diff) {
        _idx -= diff;
        return *this;
    }
    // Supports offset dereference operator ([])
    reference operator[](difference_type diff) const { return _parent->operator[](_idx + diff); }

private:
    parent_pointer  _parent;
    difference_type _idx; // not size_type because we have to use it in 'difference_type' operations most of the time
};

// ========================
// --- Helper Functions ---
// ========================

template <class T>
[[nodiscard]] std::unique_ptr<T[]> make_unique_ptr_array(size_t size) {
    return std::unique_ptr<T[]>(new T[size]);
}

// Shortuct for labda-type-based SFINAE
template <class FuncType, class Signature>
using _enable_if_signature = std::enable_if_t<std::is_convertible_v<FuncType, std::function<Signature>>, bool>;

// Variadic stringification
template <typename... Args>
[[nodiscard]] std::string _stringify(const Args&... args) {
    std::ostringstream ss;
    (ss << ... << args);
    return ss.str();
}

template <typename T>
[[nodiscard]] std::string default_stringifier(const T& value) {
    std::ostringstream ss;
    ss << std::right << std::boolalpha << value;
    return ss.str();
} // TODO: Check if that function has any reason to exist.

// Marker for uncreachable code
[[noreturn]] inline void _unreachable() {
// (Implementation from https://en.cppreference.com/w/cpp/utility/unreachable)
// Use compiler specific extensions if possible.
// Even if no extension is used, undefined behavior is still raised by
// an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
    __assume(false);
#else // GCC, Clang
    __builtin_unreachable();
#endif
}

// ======================================
// --- Sparse Matrix Pairs & Triplets ---
// ======================================

template <typename T>
struct SparseEntry1D {
    size_t i;
    T      value;
};

template <typename T>
struct SparseEntry2D {
    size_t i;
    size_t j;
    T      value;
};

template <typename T>
[[nodiscard]] bool _sparse_entry_2d_ordering(const SparseEntry2D<T>& left, const SparseEntry2D<T>& right) {
    return (left.i < right.i) || (left.j < right.j);
}

struct Index2D {
    size_t i;
    size_t j;

    bool operator==(const Index2D& other) const noexcept { return (this->i == other.i) && (this->j == other.j); }
};

[[nodiscard]] inline bool _index_2d_sparse_ordering(const Index2D& l, const Index2D& r) noexcept {
    return (l.i < r.i) || (l.j < r.j);
}

// Shortcut template used to deduce type of '_data' based on ownership inside mixins
template <Ownership ownership, typename ContainerResult, typename ViewResult, typename ConstViewResult>
using _choose_based_on_ownership =
    std::conditional_t<ownership == Ownership::CONTAINER, ContainerResult,
                       std::conditional_t<ownership == Ownership::VIEW, ViewResult, ConstViewResult>>;

// ===================
// --- Type Traits ---
// ===================

template <typename Type, typename = void>
struct _supports_stream_output : std::false_type {};

template <typename Type>
struct _supports_stream_output<Type, std::void_t<decltype(std::declval<std::ostream>() << std::declval<Type>())>>
    : std::true_type {};

// While it'd be nice to avoid macro usage alltogether, having a few macros for generating standardized boilerplate
// that gets repeated several dosen times DRASTICALLY improves the maintainability of the whole conditional compilation
// mechanism down the line. They will be later #undef'ed.

#define _utl_define_operator_support_type_trait(trait_name_, operator_)                                                \
    template <typename Type, typename = void>                                                                          \
    struct trait_name_ : std::false_type {};                                                                           \
                                                                                                                       \
    template <typename Type>                                                                                           \
    struct trait_name_<Type, std::void_t<decltype(std::declval<Type>() operator_ std::declval<Type>())>>               \
        : std::true_type {};                                                                                           \
                                                                                                                       \
    static_assert(true)

_utl_define_operator_support_type_trait(_supports_addition, +);
_utl_define_operator_support_type_trait(_supports_multiplication, *);
_utl_define_operator_support_type_trait(_supports_comparison, <);
_utl_define_operator_support_type_trait(_supports_eq_comparison, ==);

#define _utl_define_unary_operator_support_type_trait(trait_name_, operator_)                                          \
    template <typename Type, typename = void>                                                                          \
    struct trait_name_ : std::false_type {};                                                                           \
                                                                                                                       \
    template <typename Type>                                                                                           \
    struct trait_name_<Type, std::void_t<decltype(operator_ std::declval<Type>())>> : std::true_type {};               \
                                                                                                                       \
    static_assert(true)

_utl_define_unary_operator_support_type_trait(_supports_unary_minus, -);

// =====================================
// --- Boilerplate Generation Macros ---
// =====================================

#define _utl_template_arg_defs                                                                                         \
    class T, Dimension _dimension, Type _type, Ownership _ownership, Checking _checking, Layout _layout

#define _utl_template_arg_vals T, _dimension, _type, _ownership, _checking, _layout

#define _utl_require(condition_)                                                                                       \
    typename value_type = T, Dimension dimension = _dimension, Type type = _type, Ownership ownership = _ownership,    \
             Checking checking = _checking, Layout layout = _layout, std::enable_if_t<condition_, bool> = true

#define _utl_reqs(condition_) template <_utl_require(condition_)>

// ===========================
// --- Data Member Classes ---
// ===========================

// Unlike class method, member values can't be templated, which prevents us from using regular 'enable_if_t' SFINAE
// for their conditional compilation. The (seemingly) best workaround to compile members conditionally is to inherit
// 'std::contidional<T, EmptyClass>' where 'T' is a "dummy" class with the sole purpose of having data members to
// inherit. This does not introduce virtualiztion (which is good, that how we want it).

template <int id>
struct _nothing {};

template <_utl_template_arg_defs>
class _2d_extents {
private:
    using size_type = typename _types<T>::size_type;

public:
    size_type _rows = 0;
    size_type _cols = 0;
};

template <_utl_template_arg_defs>
class _2d_strides {
private:
    using size_type = typename _types<T>::size_type;

public:
    size_type _row_stride = 0;
    size_type _col_stride = 0;
};

template <_utl_template_arg_defs>
struct _2d_dense_data {
private:
    using value_type = typename _types<T>::value_type;
    using _data_t    = _choose_based_on_ownership<_ownership, std::unique_ptr<value_type[]>, _observer_ptr<value_type>,
                                               _observer_ptr<const value_type>>;

public:
    _data_t _data;
};

template <_utl_template_arg_defs>
struct _2d_sparse_data {
private:
    using value_type = typename _types<T>::value_type;
    using _triplet_t = _choose_based_on_ownership<_ownership, SparseEntry2D<value_type>,
                                                  SparseEntry2D<std::reference_wrapper<value_type>>,
                                                  SparseEntry2D<std::reference_wrapper<const value_type>>>;

public:
    using triplet_type = _triplet_t;

    std::vector<triplet_type> _data;
};

// ===================
// --- Tensor Type ---
// ===================

template <_utl_template_arg_defs>
class GenericTensor
    // Conditionally compile member variables through inheritance
    : public std::conditional_t<_dimension == Dimension::MATRIX, _2d_extents<_utl_template_arg_vals>, _nothing<1>>,
      public std::conditional_t<_dimension == Dimension::MATRIX && _type == Type::STRIDED,
                                _2d_strides<_utl_template_arg_vals>, _nothing<2>>,
      public std::conditional_t<_type == Type::DENSE || _type == Type::STRIDED, _2d_dense_data<_utl_template_arg_vals>,
                                _nothing<3>>,
      public std::conditional_t<_type == Type::SPARSE, _2d_sparse_data<_utl_template_arg_vals>, _nothing<4>>
// > After this point no non-static member variables will be introduced
{
    // --- Parameter reflection ---
    // ----------------------------
public:
    struct params {
        constexpr static auto dimension = _dimension;
        constexpr static auto type      = _type;
        constexpr static auto ownership = _ownership;
        constexpr static auto checking  = _checking;
        constexpr static auto layout    = _layout;

        // Prevent impossible layouts
        static_assert((dimension == Dimension::VECTOR) == (layout == Layout::FLAT), "Flat layout <=> matrix is 1D.");
        static_assert((type == Type::SPARSE) == (layout == Layout::SPARSE), "Sparse layout <=> matrix is sparse.");
    };

    // --- Member types ---
    // --------------------
private:
    using _type_wrapper = _types<T>;

public:
    using self            = GenericTensor;
    using value_type      = typename _type_wrapper::value_type;
    using size_type       = typename _type_wrapper::size_type;
    using difference_type = typename _type_wrapper::difference_type;
    using reference       = typename _type_wrapper::reference;
    using const_reference = typename _type_wrapper::const_reference;
    using pointer         = typename _type_wrapper::pointer;
    using const_pointer   = typename _type_wrapper::const_pointer;

    // --- Iterators ---
    // -----------------
public:
    using const_iterator         = _flat_iterator<const self*, _type_wrapper, true>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using iterator = std::conditional_t<_ownership != Ownership::CONST_VIEW, _flat_iterator<self*, _type_wrapper>,
                                        const_iterator>; // for const views 'iterator' is the same as 'const_iterator'
    using reverse_iterator = std::reverse_iterator<iterator>;

    [[nodiscard]] const_iterator cbegin() const { return const_iterator(this, 0); }
    [[nodiscard]] const_iterator cend() const { return const_iterator(this, this->size()); }
    [[nodiscard]] const_iterator begin() const { return this->cbegin(); }
    [[nodiscard]] const_iterator end() const { return this->cend(); }

    [[nodiscard]] const_reverse_iterator crbegin() const { return const_reverse_iterator(this->cend()); }
    [[nodiscard]] const_reverse_iterator crend() const { return const_reverse_iterator(this->cbegin()); }
    [[nodiscard]] const_reverse_iterator rbegin() const { return this->crbegin(); }
    [[nodiscard]] const_reverse_iterator rend() const { return this->crend(); }

    _utl_reqs(ownership != Ownership::CONST_VIEW)
    [[nodiscard]] iterator begin() { return iterator(this, 0); }

    _utl_reqs(ownership != Ownership::CONST_VIEW)
    [[nodiscard]] iterator end() { return iterator(this, this->size()); }

    _utl_reqs(ownership != Ownership::CONST_VIEW)
    [[nodiscard]] reverse_iterator rbegin() { return reverse_iterator(this->end()); }

    _utl_reqs(ownership != Ownership::CONST_VIEW)
    [[nodiscard]] reverse_iterator rend() { return reverse_iterator(this->begin()); }

    // --- Basic getters ---
    // ---------------------
public:
    _utl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] size_type size() const noexcept { return this->rows() * this->cols(); }

    _utl_reqs(type == Type::SPARSE)
    [[nodiscard]] size_type size() const noexcept { return this->_data.size(); }

    _utl_reqs(dimension == Dimension::MATRIX)
    [[nodiscard]] size_type rows() const noexcept { return this->_rows; }

    _utl_reqs(dimension == Dimension::MATRIX)
    [[nodiscard]] size_type cols() const noexcept { return this->_cols; }

    _utl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE)
    [[nodiscard]] constexpr size_type row_stride() const noexcept {
        if constexpr (self::params::layout == Layout::RC) return 0;
        if constexpr (self::params::layout == Layout::CR) return 1;
        _unreachable();
    }

    _utl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE)
    [[nodiscard]] constexpr size_type col_stride() const noexcept {
        if constexpr (self::params::layout == Layout::RC) return 1;
        if constexpr (self::params::layout == Layout::CR) return 0;
        _unreachable();
    }
    _utl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED)
    [[nodiscard]] size_type row_stride() const noexcept { return this->_row_stride; }

    _utl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED)
    [[nodiscard]] size_type col_stride() const noexcept { return this->_col_stride; }

    _utl_reqs(type == Type::DENSE || type == Type::STRIDED)
    [[nodiscard]] const_pointer data() const noexcept { return this->_data.get(); }

    _utl_reqs(ownership != Ownership::CONST_VIEW && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] pointer data() noexcept { return this->_data.get(); }

    [[nodiscard]] bool empty() const noexcept { return (this->size() == 0); }

    // --- Advanced getters ---
    // ------------------------
    _utl_reqs(_supports_eq_comparison<value_type>::value)
    [[nodiscard]] bool contains(const_reference value) const {
        return std::find(this->cbegin(), this->cend(), value) != this->cend();
    }

    _utl_reqs(_supports_eq_comparison<value_type>::value)
    [[nodiscard]] size_type count(const_reference value) const {
        return std::count(this->cbegin(), this->cend(), value);
    }

    _utl_reqs(_supports_comparison<value_type>::value)
    [[nodiscard]] bool is_sorted() const { return std::is_sorted(this->cbegin(), this->cend()); }

    template <typename Compare>
    [[nodiscard]] bool is_sorted(Compare cmp) const {
        return std::is_sorted(this->cbegin(), this->cend(), cmp);
    }

    [[nodiscard]] std::vector<value_type> to_std_vector() const { return std::vector(this->cbegin(), this->cend()); }

    _utl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE)
    self transposed() const {
        self res(this->cols(), this->rows());
        this->for_each([&](const value_type& element, size_type i, size_type j) { res(j, i) = element; });
        return res;
    }

    _utl_reqs(ownership == Ownership::CONTAINER)
    [[nodiscard]] self clone() const { return *this; }

    _utl_reqs(ownership == Ownership::CONTAINER)
    [[nodiscard]] self move() & { return std::move(*this); }

    template <Type other_type, Ownership other_ownership, Checking other_checking, Layout other_layout>
    [[nodiscard]] bool
    compare_contents(const GenericTensor<value_type, self::params::dimension, other_type, other_ownership,
                                         other_checking, other_layout>& other) const {
        // Surface-level checks
        if ((this->rows() != other.rows()) || (this->cols() != other.cols())) return false;
        // Compare while respecting sparsity
        constexpr bool is_sparse_l = (self::params::type == Type::SPARSE);
        constexpr bool is_sparse_r = (std::remove_reference_t<decltype(other)>::params::type == Type::SPARSE);
        // Same sparsity comparison
        if constexpr (is_sparse_l == is_sparse_r) {
            return this->size() == other.size() &&
                   this->true_for_all([&](const_reference e, size_type i, size_type j) { return e == other(i, j); });
        }
        // Different sparsity comparison
        // TODO: Impl here and use .all_of() OR .any_of()
        return true;
    }

    // --- Indexation ---
    // ------------------
public:
    // Vector API
    [[nodiscard]] const_reference front() const { return this->operator[](0); }
    [[nodiscard]] const_reference back() const { return this->operator[](this->size() - 1); }
    [[nodiscard]] reference       front() { return this->operator[](0); }
    [[nodiscard]] reference       back() { return this->operator[](this->size() - 1); }

private:
    _utl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED)
    [[nodiscard]] size_type _get_memory_offset_strided_impl(size_type idx, size_type i, size_type j) const {
        if constexpr (self::params::layout == Layout::RC) return idx * this->col_stride() + this->row_stride() * i;
        if constexpr (self::params::layout == Layout::CR) return idx * this->row_stride() + this->col_stride() * j;
        _unreachable();
    }

public:
    _utl_reqs(type == Type::DENSE)
    [[nodiscard]] size_type get_memory_offset_of_idx(size_type idx) const {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(idx);
        return idx;
    }

    _utl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE)
    [[nodiscard]] size_type get_memory_offset_of_ij(size_type i, size_type j) const {
        return this->get_idx_of_ij(i, j);
    }

    _utl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED)
    [[nodiscard]] size_type get_memory_offset_of_idx(size_type idx) const {
        const auto ij = this->get_ij_of_idx(idx);
        return _get_memory_offset_strided_impl(idx, ij.i, ij.j);
    }

    _utl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED)
    [[nodiscard]] size_type get_memory_offset_of_ij(size_type i, size_type j) const {
        const auto idx = this->get_idx_of_ij(i, j);
        return _get_memory_offset_strided_impl(idx, i, j);
    }

public:
    // - Flat indexation -
    _utl_reqs(ownership != Ownership::CONST_VIEW && dimension == Dimension::MATRIX &&
              (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] reference operator[](size_type idx) { return this->data()[this->get_memory_offset_of_idx(idx)]; }

    _utl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] const_reference operator[](size_type idx) const {
        return this->data()[this->get_memory_offset_of_idx(idx)];
    }

    _utl_reqs(ownership != Ownership::CONST_VIEW && dimension == Dimension::VECTOR || type == Type::SPARSE)
    [[nodiscard]] reference operator[](size_type idx) {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(idx);
        return this->_data[idx].value;
    }

    _utl_reqs(dimension == Dimension::VECTOR || type == Type::SPARSE)
    [[nodiscard]] const_reference operator[](size_type idx) const {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(idx);
        return this->_data[idx].value;
    }

    // - 2D indexation -
    _utl_reqs(ownership != Ownership::CONST_VIEW && dimension == Dimension::MATRIX &&
              (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] reference operator()(size_type i, size_type j) {
        return this->data()[this->get_memory_offset_of_ij(i, j)];
    }

    _utl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] const_reference operator()(size_type i, size_type j) const {
        return this->data()[this->get_memory_offset_of_ij(i, j)];
    }

    _utl_reqs(ownership != Ownership::CONST_VIEW && dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] reference operator()(size_type i, size_type j) {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(i, j);
        return this->_data[this->get_idx_of_ij(i, j)].value;
    }
    _utl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] const_reference operator()(size_type i, size_type j) const {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(i, j);
        return this->_data[this->get_idx_of_ij(i, j)].value;
    }

    // --- Index conversions ---
    // -------------------------

    // - Bound checking -
private:
    void _bound_check_idx(size_type idx) const {
        if (idx >= this->size())
            throw std::out_of_range(
                _stringify("idx (which is ", idx, ") >= this->size() (which is ", this->size(), ")"));
    }

    _utl_reqs(dimension == Dimension::MATRIX)
    void _bound_check_ij(size_type i, size_type j) const {
        if (i >= this->rows())
            throw std::out_of_range(_stringify("i (which is ", i, ") >= this->rows() (which is ", this->rows(), ")"));
        else if (j >= this->cols())
            throw std::out_of_range(_stringify("j (which is ", j, ") >= this->cols() (which is ", this->cols(), ")"));
    }

    // - Dense & strided implementations -
private:
    _utl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] size_type _unchecked_get_idx_of_ij(size_type i, size_type j) const {
        if constexpr (self::params::layout == Layout::RC) return i * this->cols() + j;
        if constexpr (self::params::layout == Layout::CR) return j * this->rows() + i;
        _unreachable();
    }

    _utl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] Index2D _unchecked_get_ij_of_idx(size_type idx) const {
        if constexpr (self::params::layout == Layout::RC) return {idx / this->cols(), idx % this->cols()};
        if constexpr (self::params::layout == Layout::CR) return {idx % this->rows(), idx / this->rows()};
        _unreachable();
    }

    _utl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::CONTAINER)
    [[nodiscard]] size_type _total_allocated_size() const noexcept {
        // Note 1: Allocated size of the strided matrix is NOT equal to .size() (which is same as rows * cols)
        // This is due to all the padding between the actual elements
        // Note 2: The question of whether .size() should return the number of 'strided' elements or the number
        // of actually allocated elements is rather perplexing, while the second option is more "correct" its
        // usefulness is dubious at bets, while the first one does in fact allow convenient 1D iteration over
        // all the elements. Option 1 also provides an API consistent with strided views, which is why it ended
        // up being the one chosen
        //
        // This method returns a true allocated size
        if constexpr (self::params::layout == Layout::RC)
            return (this->rows() - 1) * this->row_stride() + this->rows() * this->cols() * this->col_stride();
        if constexpr (self::params::layout == Layout::CR)
            return (this->cols() - 1) * this->col_stride() + this->rows() * this->cols() * this->row_stride();
        _unreachable();
    }

public:
    _utl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] size_type get_idx_of_ij(size_type i, size_type j) const {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_ij(i, j);
        return _unchecked_get_idx_of_ij(i, j);
    }

    _utl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] Index2D get_ij_of_idx(size_type idx) const {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(idx);
        return _unchecked_get_ij_of_idx(idx);
    }

    _utl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] size_type extent_major() const noexcept {
        if constexpr (self::params::layout == Layout::RC) return this->rows();
        if constexpr (self::params::layout == Layout::CR) return this->cols();
        _unreachable();
    }

    _utl_reqs(dimension == Dimension::MATRIX && (type == Type::DENSE || type == Type::STRIDED))
    [[nodiscard]] size_type extent_minor() const noexcept {
        if constexpr (self::params::layout == Layout::RC) return this->cols();
        if constexpr (self::params::layout == Layout::CR) return this->rows();
        _unreachable();
    }

    // - Sparse implementations -
private:
    _utl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] size_type _search_ij(size_type i, size_type j) const noexcept {
        // Returns this->size() if {i, j} wasn't found.
        // Linear search for small .size() (more efficient fue to prediction and cache locality)
        if (true) {
            for (size_type idx = 0; idx < this->size(); ++idx)
                if (this->_data[idx].i == i && this->_data[idx].j == j) return idx;
            return this->size();
        }
        // TODO: Binary search for larger .size() (N(log2(size)) instead of N(size) asymptotically)
    }

public:
    _utl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] size_type get_idx_of_ij(size_type i, size_type j) const {
        const size_type idx = this->_search_ij(i, j);
        // Return this->size() if {i, j} wasn't found. Throw with bound checking.
        if constexpr (self::params::checking == Checking::BOUNDS)
            if (idx == this->size())
                throw std::out_of_range(_stringify("Index { ", i, ", ", j, "} in not a part of sparse matrix"));
        return idx;
    }

    _utl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] Index2D get_ij_of_idx(size_type idx) const {
        if constexpr (self::params::checking == Checking::BOUNDS) this->_bound_check_idx(idx);
        return Index2D{this->_data[idx].i, this->_data[idx].j};
    }

    _utl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] bool contains_index(size_type i, size_type j) const noexcept {
        return this->_search_ij(i, j) != this->size();
    }

    // --- Reductions ---
    // ------------------
    _utl_reqs(_supports_addition<value_type>::value)
    [[nodiscard]] value_type sum() const { return std::accumulate(this->cbegin(), this->cend(), value_type()); }

    _utl_reqs(_supports_multiplication<value_type>::value)
    [[nodiscard]] value_type product() const {
        return std::accumulate(this->cbegin(), this->cend(), value_type(), std::multiplies<value_type>());
    }

    _utl_reqs(_supports_comparison<value_type>::value)
    [[nodiscard]] value_type min() const { return *std::min_element(this->cbegin(), this->cend()); }

    _utl_reqs(_supports_comparison<value_type>::value)
    [[nodiscard]] value_type max() const { return *std::max_element(this->cbegin(), this->cend()); }

    // --- Predicate operations ---
    // ----------------------------
    template <class PredType, _enable_if_signature<PredType, bool(const_reference)> = true>
    [[nodiscard]] bool true_for_any(PredType predicate) const {
        for (size_type idx = 0; idx < this->size(); ++idx)
            if (predicate(this->operator[](idx), idx)) return true;
        return false;
    }

    template <class PredType, _enable_if_signature<PredType, bool(const_reference, size_type)> = true>
    [[nodiscard]] bool true_for_any(PredType predicate) const {
        for (size_type idx = 0; idx < this->size(); ++idx)
            if (predicate(this->operator[](idx), idx)) return true;
        return false;
    }

    template <class PredType, _enable_if_signature<PredType, bool(const_reference, size_type, size_type)> = true,
              _utl_require(dimension == Dimension::MATRIX)>
              [[nodiscard]] bool true_for_any(PredType predicate) const {
        // Loop over all 2D indices using 1D loop with idx->ij conversion
        // This is just as fast and ensures looping only over existing elements in non-dense matrices
        for (size_type idx = 0; idx < this->size(); ++idx) {
            const auto ij = this->get_ij_of_idx(idx);
            if (predicate(this->operator[](idx), ij.i, ij.j)) return true;
        }
        return false;
    }

    template <class PredType, _enable_if_signature<PredType, bool(const_reference)> = true>
    [[nodiscard]] bool true_for_all(PredType predicate) const {
        auto inversed_predicate = [&](const_reference e) -> bool { return !predicate(e); };
        return !this->true_for_any(inversed_predicate);
    }

    template <class PredType, _enable_if_signature<PredType, bool(const_reference, size_type)> = true>
    [[nodiscard]] bool true_for_all(PredType predicate) const {
        auto inversed_predicate = [&](const_reference e, size_type idx) -> bool { return !predicate(e, idx); };
        return !this->true_for_any(inversed_predicate);
    }

    template <class PredType, _enable_if_signature<PredType, bool(const_reference, size_type, size_type)> = true,
              _utl_require(dimension == Dimension::MATRIX)>
              [[nodiscard]] bool true_for_all(PredType predicate) const {
        // We can reuse .true_for_any() with inverted predicate due to following conjecture:
        // FOR_ALL (predicate)  ~  ! FOR_ANY (!predicate)
        auto inversed_predicate = [&](const_reference e, size_type i, size_type j) -> bool {
            return !predicate(e, i, j);
        };
        return !this->true_for_any(inversed_predicate);
    }

    // --- Const algorithms ---
    // ------------------------
    template <class FuncType, _enable_if_signature<FuncType, void(const_reference)> = true>
    const self& for_each(FuncType func, _for_each_tag = _for_each_tag::DUMMY) const {
        for (size_type idx = 0; idx < this->size(); ++idx) func(this->operator[](idx));
        return *this;
    }

    template <class FuncType, _enable_if_signature<FuncType, void(const_reference, size_type)> = true>
    const self& for_each(FuncType func, _for_each_idx_tag = _for_each_idx_tag::DUMMY) const {
        for (size_type idx = 0; idx < this->size(); ++idx) func(this->operator[](idx), idx);
        return *this;
    }

    template <class FuncType, _enable_if_signature<FuncType, void(const_reference, size_type, size_type)> = true,
              _utl_require(dimension == Dimension::MATRIX)>
              const self& for_each(FuncType func) const {
        // Loop over all 2D indices using 1D loop with idx->ij conversion.
        // This is just as fast and ensures looping only over existing elements in non-dense matrices.
        for (size_type idx = 0; idx < this->size(); ++idx) {
            const auto ij = this->get_ij_of_idx(idx);
            func(this->operator[](idx), ij.i, ij.j);
        }
        return *this;
    }

    // --- Mutating algorithms ---
    // ---------------------------
    template <class FuncType, _enable_if_signature<FuncType, void(reference)> = true,
              _utl_require(ownership != Ownership::CONST_VIEW)>
              self& for_each(FuncType func) {
        for (size_type idx = 0; idx < this->size(); ++idx) func(this->operator[](idx));
        return *this;
    }

    template <class FuncType, _enable_if_signature<FuncType, void(reference, size_type)> = true,
              _utl_require(ownership != Ownership::CONST_VIEW)>
              self& for_each(FuncType func) {
        for (size_type idx = 0; idx < this->size(); ++idx) func(this->operator[](idx), idx);
        return *this;
    }

    template <class FuncType, _enable_if_signature<FuncType, void(reference, size_type, size_type)> = true,
              _utl_require(dimension == Dimension::MATRIX)>
              self& for_each(FuncType func, _for_each_ij_tag = _for_each_ij_tag::DUMMY) {
        for (size_type idx = 0; idx < this->size(); ++idx) {
            const auto ij = this->get_ij_of_idx(idx);
            func(this->operator[](idx), ij.i, ij.j);
        }
        return *this;
    }

    template <class FuncType, _enable_if_signature<FuncType, value_type(const_reference)> = true,
              _utl_require(ownership != Ownership::CONST_VIEW)>
              self& transform(FuncType func) {
        const auto func_wrapper = [&](reference elem) { elem = func(elem); };
        return this->for_each(func_wrapper);
    }

    template <class FuncType, _enable_if_signature<FuncType, value_type(const_reference, size_type)> = true,
              _utl_require(dimension == Dimension::VECTOR && ownership != Ownership::CONST_VIEW)>
              self& transform(FuncType func) {
        const auto func_wrapper = [&](reference elem, size_type i) { elem = func(elem, i); };
        return this->for_each(func_wrapper);
    }

    template <class FuncType, _enable_if_signature<FuncType, value_type(const_reference, size_type, size_type)> = true,
              _utl_require(dimension == Dimension::MATRIX && ownership != Ownership::CONST_VIEW)>
              self& transform(FuncType func, _for_each_ij_tag = _for_each_ij_tag::DUMMY) {
        const auto func_wrapper = [&](reference elem, size_type i, size_type j) { elem = func(elem, i, j); };
        return this->for_each(func_wrapper);
    }

    _utl_reqs(ownership != Ownership::CONST_VIEW)
    self& fill(const_reference value) {
        for (size_type idx = 0; idx < this->size(); ++idx) this->operator[](idx) = value;
        return *this;
    }

    template <class FuncType, _enable_if_signature<FuncType, value_type()> = true,
              _utl_require(ownership != Ownership::CONST_VIEW)>
              self& fill(FuncType func) {
        const auto func_wrapper = [&](reference elem) { elem = func(); };
        return this->for_each(func_wrapper);
    }

    template <class FuncType, _enable_if_signature<FuncType, value_type(size_type)> = true,
              _utl_require(dimension == Dimension::VECTOR && ownership != Ownership::CONST_VIEW)>
              self& fill(FuncType func) {
        const auto func_wrapper = [&](reference elem, size_type i) { elem = func(i); };
        return this->for_each(func_wrapper);
    }

    template <class FuncType, _enable_if_signature<FuncType, value_type(size_type, size_type)> = true,
              _utl_require(dimension == Dimension::MATRIX && ownership != Ownership::CONST_VIEW)>
              self& fill(FuncType func) {
        const auto func_wrapper = [&](reference elem, size_type i, size_type j) { elem = func(i, j); };
        return this->for_each(func_wrapper);
    }

    template <typename Compare, _utl_require(ownership != Ownership::CONST_VIEW)>
              self& sort(Compare cmp) {
        std::sort(this->begin(), this->end(), cmp);
        return *this;
    }
    template <typename Compare, _utl_require(ownership != Ownership::CONST_VIEW)>
              self& stable_sort(Compare cmp) {
        std::stable_sort(this->begin(), this->end(), cmp);
        return *this;
    }

    _utl_reqs(ownership != Ownership::CONST_VIEW && _supports_comparison<value_type>::value)
    self& sort() {
        std::sort(this->begin(), this->end());
        return *this;
    }
    _utl_reqs(ownership != Ownership::CONST_VIEW && _supports_comparison<value_type>::value)
    self& stable_sort() {
        std::stable_sort(this->begin(), this->end());
        return *this;
    }

    // --- Sparse Subviews ---
    // -----------------------

    // - Const views -
private:
    using _cref_triplet_array =
        std::vector<SparseEntry2D<std::reference_wrapper<const value_type>>>; // NOTE: Generalize for 1D

public:
    using sparse_const_view_type = GenericTensor<value_type, self::params::dimension, Type::SPARSE,
                                                 Ownership::CONST_VIEW, self::params::checking, Layout::SPARSE>;

    template <typename UnaryPredicate, _enable_if_signature<UnaryPredicate, bool(const_reference)> = true>
    [[nodiscard]] sparse_const_view_type filter(UnaryPredicate predicate) const {
        const auto forwarded_predicate = [&](const_reference elem, size_type, size_type) -> bool {
            return predicate(elem);
        };
        return this->filter(forwarded_predicate);
        // NOTE: This would need its own implementation for a proper 1D support
    }

    template <typename UnaryPredicate, _enable_if_signature<UnaryPredicate, bool(const_reference, size_type)> = true>
    [[nodiscard]] sparse_const_view_type filter(UnaryPredicate predicate) const {
        const auto forwarded_predicate = [&](const_reference elem, size_type i, size_type j) -> bool {
            const size_type idx = this->get_idx_of_ij(i, j);
            return predicate(elem, idx);
        };
        return this->filter(forwarded_predicate);
        // NOTE: This would need its own implementation for a proper 1D support
    }

    template <typename UnaryPredicate,
              _enable_if_signature<UnaryPredicate, bool(const_reference, size_type, size_type)> = true,
              _utl_require(dimension == Dimension::MATRIX)>
              [[nodiscard]] sparse_const_view_type filter(UnaryPredicate predicate) const {
        // We can't preallocate triplets without scanning predicate through the whole matrix,
        // so we just push back entries into a vector and use to construct a sparse view
        _cref_triplet_array triplets;
        const auto          add_triplet_if_predicate = [&](const_reference elem, size_type i, size_type j) -> void {
            if (predicate(elem, i, j)) triplets.push_back({i, j, elem});
        };

        this->for_each(add_triplet_if_predicate);

        triplets.shrink_to_fit();
        return sparse_const_view_type(this->rows(), this->cols(), std::move(triplets));
    }

    _utl_reqs(dimension == Dimension::MATRIX)
    [[nodiscard]] sparse_const_view_type diagonal() const {
        // Sparse matrices have no better way of getting a diagonal than filtering (i ==j)
        if constexpr (self::params::type == Type::SPARSE) {
            return this->filter([](const_reference, size_type i, size_type j) { return i == j; });
        }
        // Non-sparce matrices can just iterate over diagonal directly
        else {
            const size_type     min_size = std::min(this->rows(), this->cols());
            _cref_triplet_array triplets;
            triplets.reserve(min_size);
            for (size_type k = 0; k < min_size; ++k) triplets.push_back({k, k, this->operator()(k, k)});
            return sparse_const_view_type(this->rows(), this->cols(), std::move(triplets));
        }
    }

    // - Mutable views -
private:
    using _ref_triplet_array =
        std::vector<SparseEntry2D<std::reference_wrapper<value_type>>>; // NOTE: Generalize for 1D

public:
    using sparse_view_type = GenericTensor<value_type, self::params::dimension, Type::SPARSE, Ownership::VIEW,
                                           self::params::checking, Layout::SPARSE>;

    template <typename UnaryPredicate, _utl_require(ownership != Ownership::CONST_VIEW),
                                                    _enable_if_signature<UnaryPredicate, bool(const_reference)> = true>
              [[nodiscard]] sparse_view_type filter(UnaryPredicate predicate) {
        const auto forwarded_predicate = [&](const_reference elem, size_type, size_type) -> bool {
            return predicate(elem);
        };
        return this->filter(forwarded_predicate);
        // NOTE: This would need its own implementation for a proper 1D support
    }

    template <typename UnaryPredicate,
              _utl_require(ownership != Ownership::CONST_VIEW),
                           _enable_if_signature<UnaryPredicate, bool(const_reference, size_type)> = true>
              [[nodiscard]] sparse_view_type filter(UnaryPredicate predicate) {
        const auto forwarded_predicate = [&](const_reference elem, size_type i, size_type j) -> bool {
            const size_type idx = this->get_idx_of_ij(i, j);
            return predicate(elem, idx);
        };
        return this->filter(forwarded_predicate);
        // NOTE: This would need its own implementation for a proper 1D support
    }

    template <typename UnaryPredicate,
              _enable_if_signature<UnaryPredicate, bool(const_reference, size_type, size_type)> = true,
              _utl_require(dimension == Dimension::MATRIX && ownership != Ownership::CONST_VIEW)>
              [[nodiscard]] sparse_view_type filter(UnaryPredicate predicate) {
        // This method implements actual filtering, others just forward predicates to it
        _ref_triplet_array triplets;
        // We can't preallocate triplets without scanning predicate through the whole matrix,
        // so we just push back entries into a vector and use to construct a sparse view
        const auto         add_triplet_if_predicate = [&](reference elem, size_type i, size_type j) -> void {
            if (predicate(elem, i, j)) triplets.push_back({i, j, elem});
        };

        this->for_each(add_triplet_if_predicate);

        triplets.shrink_to_fit();
        return sparse_view_type(this->rows(), this->cols(), std::move(triplets));
    }

    _utl_reqs(dimension == Dimension::MATRIX && ownership != Ownership::CONST_VIEW)
    [[nodiscard]] sparse_view_type diagonal() {
        /* Sparse matrices have no better way of getting a diagonal than filtering (i == j) */
        if constexpr (self::params::type == Type::SPARSE) {
            return this->filter([](const_reference, size_type i, size_type j) { return i == j; });
        } /* Non-sparce matrices can just iterate over diagonal directly */
        else {
            const size_type    min_size = std::min(this->rows(), this->cols());
            _ref_triplet_array triplets;
            triplets.reserve(min_size);
            for (size_type k = 0; k < min_size; ++k) triplets.push_back({k, k, this->operator()(k, k)});
            return sparse_view_type(this->rows(), this->cols(), std::move(triplets));
        }
    }

    // --- Block Subviews ---
    // ----------------------
public:
    // - Const views -
    using block_const_view_type =
        std::conditional_t<self::params::type == Type::SPARSE, sparse_const_view_type,
                           GenericTensor<value_type, self::params::dimension, Type::STRIDED, Ownership::CONST_VIEW,
                                         self::params::checking, self::params::layout>>;

    _utl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    [[nodiscard]] block_const_view_type block(size_type block_i, size_type block_j, size_type block_rows,
                                              size_type block_cols) const {
        // Sparse matrices have no better way of getting a block than filtering by { i, j }

        // Do the same thing as in .filter(), but shrink resulting view size to
        // { block_rows, block_cols } and shift indexation by { block_i, block_j }
        _cref_triplet_array triplets;

        const auto add_triplet_if_inside_block = [&](const_reference elem, size_type i, size_type j) -> void {
            if ((block_i <= i) && (i < block_i + block_rows) && (block_j <= j) && (j < block_j + block_cols))
                triplets.push_back({i - block_i, j - block_j, elem});
        };

        this->for_each(add_triplet_if_inside_block);

        triplets.shrink_to_fit();
        return block_const_view_type(block_rows, block_cols, std::move(triplets));
    }

    _utl_reqs(dimension == Dimension::MATRIX && type != Type::SPARSE)
    [[nodiscard]] block_const_view_type block(size_type block_i, size_type block_j, size_type block_rows,
                                              size_type block_cols) const {
        if constexpr (self::params::layout == Layout::RC) {
            const size_type row_stride = this->row_stride() + this->col_stride() * (this->cols() - block_cols);
            const size_type col_stride = this->col_stride();
            return block_const_view_type(block_rows, block_cols, row_stride, col_stride,
                                         &this->operator()(block_i, block_j));
        }
        if constexpr (self::params::layout == Layout::CR) {
            const size_type row_stride = this->row_stride();
            const size_type col_stride = this->col_stride() + this->row_stride() * (this->rows() - block_rows);
            return block_const_view_type(block_rows, block_cols, row_stride, col_stride,
                                         &this->operator()(block_i, block_j));
        }
        _unreachable();
    }

    _utl_reqs(dimension == Dimension::MATRIX)
    [[nodiscard]] block_const_view_type row(size_type i) const { return this->block(i, 0, 1, this->cols()); }

    _utl_reqs(dimension == Dimension::MATRIX)
    [[nodiscard]] block_const_view_type col(size_type j) const { return this->block(0, j, this->rows(), 1); }

    // - Mutable views -
    using block_view_type =
        std::conditional_t<self::params::type == Type::SPARSE, sparse_view_type,
                           GenericTensor<value_type, self::params::dimension, Type::STRIDED, Ownership::VIEW,
                                         self::params::checking, self::params::layout>>;

    _utl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE && ownership != Ownership::CONST_VIEW)
    [[nodiscard]] block_view_type block(size_type block_i, size_type block_j, size_type block_rows,
                                        size_type block_cols) {
        // Sparse matrices have no better way of getting a block than filtering by { i, j }

        // Do the same thing as in .filter(), but shrink resulting view size to
        // { block_rows, block_cols } and shift indexation by { block_i, block_j }
        _ref_triplet_array triplets;

        const auto add_triplet_if_inside_block = [&](reference elem, size_type i, size_type j) -> void {
            if ((block_i <= i) && (i < block_i + block_rows) && (block_j <= j) && (j < block_j + block_cols))
                triplets.push_back({i - block_i, j - block_j, elem});
        };

        this->for_each(add_triplet_if_inside_block);

        triplets.shrink_to_fit();
        return block_view_type(block_rows, block_cols, std::move(triplets));
    }

    _utl_reqs(dimension == Dimension::MATRIX && type != Type::SPARSE && ownership != Ownership::CONST_VIEW)
    [[nodiscard]] block_view_type block(size_type block_i, size_type block_j, size_type block_rows,
                                        size_type block_cols) {
        if constexpr (self::params::layout == Layout::RC) {
            const size_type row_stride = this->row_stride() + this->col_stride() * (this->cols() - block_cols);
            const size_type col_stride = this->col_stride();
            return block_view_type(block_rows, block_cols, row_stride, col_stride, &this->operator()(block_i, block_j));
        }
        if constexpr (self::params::layout == Layout::CR) {
            const size_type row_stride = this->row_stride();
            const size_type col_stride = this->col_stride() + this->row_stride() * (this->rows() - block_rows);
            return block_view_type(block_rows, block_cols, row_stride, col_stride, &this->operator()(block_i, block_j));
        }
        _unreachable();
    }

    _utl_reqs(dimension == Dimension::MATRIX && ownership != Ownership::CONST_VIEW)
    [[nodiscard]] block_view_type row(size_type i) { return this->block(i, 0, 1, this->cols()); }

    _utl_reqs(dimension == Dimension::MATRIX && ownership != Ownership::CONST_VIEW)
    [[nodiscard]] block_view_type col(size_type j) { return this->block(0, j, this->rows(), 1); }

    // --- Sparse operations ---
    // -------------------------

private:
    using _triplet_t = _choose_based_on_ownership<_ownership, SparseEntry2D<value_type>,
                                                  SparseEntry2D<std::reference_wrapper<value_type>>,
                                                  SparseEntry2D<std::reference_wrapper<const value_type>>>;

public:
    using sparse_entry_type = _triplet_t;

    _utl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    self& insert_triplets(const std::vector<sparse_entry_type>& triplets) {
        // Bulk-insert triplets and sort by index
        const auto ordering = [](const sparse_entry_type& l, const sparse_entry_type& r) -> bool {
            return (l.i < r.i) && (l.j < r.j);
        };

        this->_data.insert(this->_data.end(), triplets.begin(), triplets.end());
        std::sort(this->_data.begin(), this->_data.end(), ordering);

        return *this;
    }

    _utl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    self& rewrite_triplets(std::vector<sparse_entry_type>&& triplets) {
        // Move-construct all triplets at once and sort by index
        const auto ordering = [](const sparse_entry_type& l, const sparse_entry_type& r) -> bool {
            return (l.i < r.i) && (l.j < r.j);
        };

        this->_data = std::move(triplets);
        std::sort(this->_data.begin(), this->_data.end(), ordering);

        return *this;
    }

    _utl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    self& erase_triplets(std::vector<Index2D> indices) {
        // Erase triplets with {i, j} from 'indices' using the fact that both
        // 'indices' and triplets are sorted. We can scan through triplets once
        // while advancing 'cursor' when 'indices[cursor]' gets deleted, which
        // result in all necessary triplets being marked for erasure in order.
        std::sort(indices.begin(), indices.end(), _index_2d_sparse_ordering);
        std::size_t cursor = 0;

        const auto erase_condition = [&](const sparse_entry_type& triplet) -> bool {
            /* Stop erasing once all target indices are handled */
            if (cursor == indices.size()) return false;
            if (indices[cursor].i == triplet.i && indices[cursor].j == triplet.j) {
                ++cursor;
                return true;
            }
            return false;
        };

        const auto iter = std::remove_if(this->_data.begin(), this->_data.end(), erase_condition);
        this->_data.erase(iter, this->_data.end());

        // Re-sort triplets just in case
        const auto ordering = [](const sparse_entry_type& l, const sparse_entry_type& r) -> bool {
            return (l.i < r.i) && (l.j < r.j);
        };
        std::sort(this->_data.begin(), this->_data.end(), ordering);

        return *this;
    }

    // --- Constructors ---
    // --------------------

    // - Matrix -
public:
    // Rule of five:
    // copy-ctor       - [+] (deduced from copy-assignment)
    // move-ctor       - [+] (= default)
    // copy-assignment - [+] (custom for dense/strided, same as default for sparse)
    // move-assignment - [+] (= default)
    // destructor      - [+] (= default)

    // Move-ctor is default for all types
    GenericTensor(self&& other) noexcept = default;

    // Move-assignment is default for all types
    self& operator=(self&& other) noexcept = default;

    // Copy-ctor is deduced from assignment operator
    GenericTensor(const self& other) { *this = other; }

    // Copy-assignment
    self& operator=(const self& other) {
        // Note: copy-assignment operator CANNOT be templated, it has to be implemented with 'if constexpr'
        this->_rows = other.rows();
        this->_cols = other.cols();
        if constexpr (self::params::type == Type::DENSE) {
            this->_data = std::move(make_unique_ptr_array<value_type>(this->size()));
            std::copy(other.begin(), other.end(), this->begin());
        }
        if constexpr (self::params::type == Type::STRIDED) {
            this->_row_stride = other.row_stride();
            this->_col_stride = other.col_stride();
            this->_data       = std::move(make_unique_ptr_array<value_type>(this->size()));
            std::copy(other.begin(), other.end(), this->begin());
        }
        if constexpr (self::params::type == Type::SPARSE) { this->_data = other._data; }
        return *this;
    }

    // Default-ctor (containers)
    _utl_reqs(ownership == Ownership::CONTAINER)
    GenericTensor() noexcept {}

    // Default-ctor (views)
    _utl_reqs(ownership != Ownership::CONTAINER)
    GenericTensor() noexcept = delete;

    // Copy-assignment over the config boundaries
    // We can change checking config, copy from matrices with different layouts,
    // copy from views and even matrices of other types
    template <Type other_type, Ownership other_ownership, Checking other_checking, Layout other_layout,
              _utl_require(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::CONTAINER)>
              self& operator=(const GenericTensor<value_type, self::params::dimension, other_type, other_ownership,
                                                  other_checking, other_layout>& other) {
        this->_rows = other.rows();
        this->_cols = other.cols();
        this->_data = std::move(make_unique_ptr_array<value_type>(this->size()));
        this->fill(value_type());
        other.for_each([&](const value_type& element, size_type i, size_type j) { this->operator()(i, j) = element; });
        return *this;
        // copying from sparse to dense works, all elements that weren't in the sparse matrix remain default-initialized
    }

    template <Type other_type, Ownership other_ownership, Checking other_checking, Layout other_layout,
              _utl_require(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::CONTAINER)>
              self& operator=(const GenericTensor<value_type, self::params::dimension, other_type, other_ownership,
                                                  other_checking, other_layout>& other) {
        this->_rows       = other.rows();
        this->_cols       = other.cols();
        this->_row_stride = other.row_stride();
        this->_col_strude = other.col_stride();
        this->_data       = std::move(make_unique_ptr_array<value_type>(this->size()));
        this->fill(value_type());
        // Not quite sure whether swapping strides when changing layouts like this is okay,
        // but it seems to be correct
        if constexpr (self::params::layout != other_layout) std::swap(this->_row_stride, this->_cols_stride);
        other.for_each([&](const value_type& element, size_type i, size_type j) { this->operator()(i, j) = element; });
        return *this;
        // copying from sparse to strided works, all elements that weren't in the sparse matrix remain
        // default-initialized
    }

    template <Type other_type, Ownership other_ownership, Checking other_checking, Layout other_layout,
              _utl_require(dimension == Dimension::MATRIX && type == Type::SPARSE && ownership == Ownership::CONTAINER)>
              self& operator=(const GenericTensor<value_type, self::params::dimension, other_type, other_ownership,
                                                  other_checking, other_layout>& other) {
        this->_rows = other.rows();
        this->_cols = other.cols();
        std::vector<sparse_entry_type> triplets;

        // Other sparse matrices can be trivially copied
        if constexpr (other_type == Type::SPARSE) {
            triplets.reserve(other.size());
            other.for_each([&](const value_type& elem, size_type i, size_type j) { triplets.push_back({i, j, elem}); });
        }
        // Non-sparse matrices are filtered by non-default-initialized-elements to construct a sparse subset
        else {
            other.for_each([&](const_reference elem, size_type i, size_type j) {
                if (elem != value_type()) triplets.push_back({i, j, elem});
            });
        }

        this->rewrite_triplets(std::move(triplets));
        return *this;
    }

    // Copy-ctor over the config boundaries (deduced from assignment over config boundaries)
    template <Type other_type, Ownership other_ownership, Checking other_checking, Layout other_layout,
              _utl_require(dimension == Dimension::MATRIX && ownership == Ownership::CONTAINER)>
              GenericTensor(const GenericTensor<value_type, self::params::dimension, other_type, other_ownership,
                                                other_checking, other_layout>& other) {
        *this = other;
    }

    // Move-assignment over config boundaries
    // Note: Unlike copying, we can't change layout, only checking config
    // Also 'other' can no longer be a view or have a different type
    template <Checking other_checking,
              _utl_require(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::CONTAINER)>
              self& operator=(GenericTensor<value_type, self::params::dimension, self::params::type,
                                            self::params::ownership, other_checking, self::params::layout>&& other) {
        this->_rows = other.rows();
        this->_cols = other.cols();
        this->_data = std::move(other._data);
        return *this;
    }

    template <Checking other_checking, _utl_require(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::CONTAINER)>
              self& operator=(GenericTensor<value_type, self::params::dimension, self::params::type,
                                            self::params::ownership, other_checking, self::params::layout>&& other) {
        this->_rows       = other.rows();
        this->_cols       = other.cols();
        this->_row_stride = other.row_stride();
        this->_col_stride = other.col_stride();
        this->_data       = std::move(other._data);
        return *this;
    }

    template <Checking other_checking,
              _utl_require(dimension == Dimension::MATRIX && type == Type::SPARSE && ownership == Ownership::CONTAINER)>
              self& operator=(GenericTensor<value_type, self::params::dimension, self::params::type,
                                            self::params::ownership, other_checking, self::params::layout>&& other) {
        this->_rows = other.rows();
        this->_cols = other.cols();
        this->_data = std::move(other._data);
        return *this;
    }

    // Move-ctor over the config boundaries (deduced from move-assignment over config boundaries)
    template <Type other_type, Ownership other_ownership, Checking other_checking, Layout other_layout,
              _utl_require(dimension == Dimension::MATRIX && ownership == Ownership::CONTAINER)>
              GenericTensor(GenericTensor<value_type, self::params::dimension, other_type, other_ownership,
                                          other_checking, other_layout>&& other) {
        *this = std::move(other);
    }

    // Init-with-value
    _utl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::CONTAINER)
    explicit GenericTensor(size_type rows, size_type cols, const_reference value = value_type()) noexcept {
        this->_rows = rows;
        this->_cols = cols;
        this->_data = std::move(make_unique_ptr_array<value_type>(this->size()));
        this->fill(value);
    }

    // Init-with-lambda
    template <typename FuncType,
              _utl_require(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::CONTAINER) >
              explicit GenericTensor(size_type rows, size_type cols, FuncType init_func) {
        // .fill() already takes care of preventing improper values of 'FuncType', no need to do the check here
        this->_rows = rows;
        this->_cols = cols;
        this->_data = std::move(make_unique_ptr_array<value_type>(this->size()));
        this->fill(init_func);
    }

    // Init-with-ilist
    _utl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::CONTAINER)
    GenericTensor(std::initializer_list<std::initializer_list<value_type>> init) {
        this->_rows = init.size();
        this->_cols = (*init.begin()).size();
        this->_data = std::move(make_unique_ptr_array<value_type>(this->size()));

        // Check dimensions (throw if cols have different dimensions)
        for (auto row_it = init.begin(); row_it < init.end(); ++row_it)
            if (static_cast<size_type>((*row_it).end() - (*row_it).begin()) != this->cols())
                throw std::invalid_argument("Initializer list dimensions don't match.");

        // Copy elements
        for (size_type i = 0; i < this->rows(); ++i)
            for (size_type j = 0; j < this->cols(); ++j) this->operator()(i, j) = (init.begin()[i]).begin()[j];
    }

    // Init-with-data
    _utl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::CONTAINER)
    explicit GenericTensor(size_type rows, size_type cols, pointer data_ptr) noexcept {
        this->_rows = rows;
        this->_cols = cols;
        this->_data = std::move(decltype(this->_data)(data_ptr));
    }

    // - Matrix View -

    // Init-from-data
    _utl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::VIEW)
    explicit GenericTensor(size_type rows, size_type cols, pointer data_ptr) {
        this->_rows = rows;
        this->_cols = cols;
        this->_data = data_ptr;
    }

    // Init-from-tensor (any tensor of the same API type)
    template <Ownership other_ownership, Checking other_checking, Layout other_layout,
              _utl_require(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::VIEW)>
              GenericTensor(GenericTensor<value_type, self::params::dimension, self::params::type, other_ownership,
                                          other_checking, other_layout>& other) {
        this->_rows = other.rows();
        this->_cols = other.cols();
        this->_data = other.data();
    }

    // - Const Matrix View -

    // Init-from-data
    _utl_reqs(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::CONST_VIEW)
    explicit GenericTensor(size_type rows, size_type cols, const_pointer data_ptr) {
        this->_rows = rows;
        this->_cols = cols;
        this->_data = data_ptr;
    }

    // Init-from-tensor (any tensor of the same API type)
    template <Ownership other_ownership, Checking other_checking, Layout other_layout,
              _utl_require(dimension == Dimension::MATRIX && type == Type::DENSE && ownership == Ownership::CONST_VIEW)>
              GenericTensor(const GenericTensor<value_type, self::params::dimension, self::params::type,
                                                other_ownership, other_checking, other_layout>& other) {
        this->_rows = other.rows();
        this->_cols = other.cols();
        this->_data = other.data();
    }

    // - Strided Matrix -

    // Init-with-value
    _utl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::CONTAINER)
    explicit GenericTensor(size_type rows, size_type cols, size_type row_stride, size_type col_stride,
                           const_reference value = value_type()) noexcept {
        this->_rows       = rows;
        this->_cols       = cols;
        this->_row_stride = row_stride;
        this->_col_stride = col_stride;
        // Allocates size is NOT the same as .size() due to padding, see notes on '_total_allocated_size()'
        this->_data       = std::move(make_unique_ptr_array<value_type>(this->_total_allocated_size()));
        this->fill(value);
    }

    // Init-with-lambda
    template <typename FuncType, _utl_require(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::CONTAINER) >
              explicit GenericTensor(size_type rows, size_type cols, size_type row_stride, size_type col_stride,
                                     FuncType init_func) {
        // .fill() already takes care of preventing improper values of 'FuncType', no need to do the check here
        this->_rows       = rows;
        this->_cols       = cols;
        this->_row_stride = row_stride;
        this->_col_stride = col_stride;
        // Allocates size is NOT the same as .size() due to padding, see notes on '_total_allocated_size()'
        this->_data       = std::move(make_unique_ptr_array<value_type>(this->_total_allocated_size()));
        this->fill(init_func);
    }

    // Init-with-ilist
    _utl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::CONTAINER)
    GenericTensor(std::initializer_list<std::initializer_list<value_type>> init, size_type row_stride,
                  size_type col_stride) {
        this->_rows       = init.size();
        this->_cols       = (*init.begin()).size();
        this->_row_stride = row_stride;
        this->_col_stride = col_stride;
        // Allocates size is NOT the same as .size() due to padding, see notes on '_total_allocated_size()'
        this->_data       = std::move(make_unique_ptr_array<value_type>(this->_total_allocated_size()));

        // Check dimensions (throw if cols have different dimensions)
        for (auto row_it = init.begin(); row_it < init.end(); ++row_it)
            if ((*row_it).end() - (*row_it).begin() != this->_cols)
                throw std::invalid_argument("Initializer list dimensions don't match.");

        // Copy elements
        for (size_type i = 0; i < this->rows(); ++i)
            for (size_type j = 0; j < this->cols(); ++j) this->operator()(i, j) = (init.begin()[i]).begin()[j];
    }

    // Init-with-data
    _utl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::CONTAINER)
    explicit GenericTensor(size_type rows, size_type cols, size_type row_stride, size_type col_stride,
                           pointer data_ptr) noexcept {
        this->_rows       = rows;
        this->_cols       = cols;
        this->_row_stride = row_stride;
        this->_col_stride = col_stride;
        this->_data       = std::move(decltype(this->_data)(data_ptr));
    }

    // - Strided Matrix View -

    // Init-from-data
    _utl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::VIEW)
    explicit GenericTensor(size_type rows, size_type cols, size_type row_stride, size_type col_stride,
                           pointer data_ptr) {
        this->_rows       = rows;
        this->_cols       = cols;
        this->_row_stride = row_stride;
        this->_col_stride = col_stride;
        this->_data       = data_ptr;
    }

    // Init-from-tensor (any tensor of the same API type)
    template <Ownership other_ownership, Checking other_checking, Layout other_layout,
              _utl_require(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::VIEW)>
              GenericTensor(GenericTensor<value_type, self::params::dimension, self::params::type, other_ownership,
                                          other_checking, other_layout>& other) {
        this->_rows       = other.rows();
        this->_cols       = other.cols();
        this->_row_stride = other.row_stride();
        this->_col_stride = other.col_stride();
        this->_data       = other.data();
    }

    // - Const Strided Matrix View -

    // Init-from-data
    _utl_reqs(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::CONST_VIEW)
    explicit GenericTensor(size_type rows, size_type cols, size_type row_stride, size_type col_stride,
                           const_pointer data_ptr) {
        this->_rows       = rows;
        this->_cols       = cols;
        this->_row_stride = row_stride;
        this->_col_stride = col_stride;
        this->_data       = data_ptr;
    }

    // Init-from-tensor (any tensor of the same API type)
    template <Ownership other_ownership, Checking other_checking, Layout other_layout,
              _utl_require(dimension == Dimension::MATRIX && type == Type::STRIDED && ownership == Ownership::CONST_VIEW)>
              GenericTensor(const GenericTensor<value_type, self::params::dimension, self::params::type,
                                                other_ownership, other_checking, other_layout>& other) {
        this->_rows       = other.rows();
        this->_cols       = other.cols();
        this->_row_stride = other.row_stride();
        this->_col_stride = other.col_stride();
        this->_data       = other.data();
    }

    // - Sparse Matrix / Sparse Matrix View / Sparse Matrix Const View -

    // Init-from-data (copy)
    _utl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    explicit GenericTensor(size_type rows, size_type cols, const std::vector<sparse_entry_type>& data) {
        this->_rows = rows;
        this->_cols = cols;
        this->insert_triplets(std::move(data));
    }

    // Init-from-data (move)
    _utl_reqs(dimension == Dimension::MATRIX && type == Type::SPARSE)
    explicit GenericTensor(size_type rows, size_type cols, std::vector<sparse_entry_type>&& data) {
        this->_rows = rows;
        this->_cols = cols;
        this->rewrite_triplets(std::move(data));
    }
};

// ===========================
// --- Predefined Typedefs ---
// ===========================

constexpr auto _default_checking        = Checking::NONE;
constexpr auto _default_layout_dense_2d = Layout::RC;

// - Dense 2D -
template <typename T, Checking checking = _default_checking, Layout layout = _default_layout_dense_2d>
using Matrix = GenericTensor<T, Dimension::MATRIX, Type::DENSE, Ownership::CONTAINER, checking, layout>;

template <typename T, Checking checking = _default_checking, Layout layout = _default_layout_dense_2d>
using MatrixView = GenericTensor<T, Dimension::MATRIX, Type::DENSE, Ownership::VIEW, checking, layout>;

template <typename T, Checking checking = _default_checking, Layout layout = _default_layout_dense_2d>
using ConstMatrixView = GenericTensor<T, Dimension::MATRIX, Type::DENSE, Ownership::CONST_VIEW, checking, layout>;

// - Strided 2D -
template <typename T, Checking checking = _default_checking, Layout layout = _default_layout_dense_2d>
using StridedMatrix = GenericTensor<T, Dimension::MATRIX, Type::STRIDED, Ownership::CONTAINER, checking, layout>;

template <typename T, Checking checking = _default_checking, Layout layout = _default_layout_dense_2d>
using StridedMatrixView = GenericTensor<T, Dimension::MATRIX, Type::STRIDED, Ownership::VIEW, checking, layout>;

template <typename T, Checking checking = _default_checking, Layout layout = _default_layout_dense_2d>
using ConstStridedMatrixView =
    GenericTensor<T, Dimension::MATRIX, Type::STRIDED, Ownership::CONST_VIEW, checking, layout>;

// - Sparse 2D -
template <typename T, Checking checking = _default_checking>
using SparseMatrix = GenericTensor<T, Dimension::MATRIX, Type::SPARSE, Ownership::CONTAINER, checking, Layout::SPARSE>;

template <typename T, Checking checking = _default_checking>
using SparseMatrixView = GenericTensor<T, Dimension::MATRIX, Type::SPARSE, Ownership::VIEW, checking, Layout::SPARSE>;

template <typename T, Checking checking = _default_checking>
using ConstSparseMatrixView =
    GenericTensor<T, Dimension::MATRIX, Type::SPARSE, Ownership::CONST_VIEW, checking, Layout::SPARSE>;

// ==================
// --- Formatters ---
// ==================

namespace format {

// TODO: Formats 'as_matrix', 'as_dictionary', 'as_json_array', 'as_raw_text' need 1D overloads

// - Human-readable formats -

constexpr std::size_t max_displayed_rows      = 70;
constexpr std::size_t max_displayed_cols      = 40;
constexpr std::size_t max_displayed_flat_size = 500;
constexpr auto        content_indent          = "  ";

template <class T, Dimension dimension, Type type, Ownership ownership, Checking checking, Layout layout>
[[nodiscard]] std::string
_stringify_metainfo(const GenericTensor<T, dimension, type, ownership, checking, layout>& tensor) {
    std::ostringstream ss;
    ss << "Tensor [size = " << tensor.size() << "] (" << tensor.rows() << " x " << tensor.cols() << "):\n";
    return ss.str();
}

template <class T, Dimension dimension, Type type, Ownership ownership, Checking checking, Layout layout>
[[nodiscard]] std::string _as_too_large(const GenericTensor<T, dimension, type, ownership, checking, layout>& tensor) {
    std::ostringstream ss;
    ss << _stringify_metainfo(tensor) << content_indent << "<hidden due to large size>\n";
    return ss.str();
}

template <class T>
[[nodiscard]] std::string _ss_stringify(const T& value) {
    std::ostringstream ss;
    ss.flags(std::ios::boolalpha);
    ss << value;
    return ss.str();
}

template <class T>
[[nodiscard]] std::string _ss_stringify_for_json(const T& value) {
    // Modification of '_ss_stringify()' that properly handles floats for JSON
    std::ostringstream ss;
    ss.flags(std::ios::boolalpha);

    if constexpr (std::is_floating_point_v<T>) {
        if (std::isfinite(value)) ss << value;
        else ss << "\"" << value << "\"";
    } else {
        ss << value;
    }

    return ss.str();
}

template <class T, Dimension dimension, Type type, Ownership ownership, Checking checking, Layout layout>
[[nodiscard]] std::string as_vector(const GenericTensor<T, dimension, type, ownership, checking, layout>& tensor) {
    if (tensor.size() > max_displayed_flat_size) return _as_too_large(tensor);

    std::ostringstream ss;
    ss << _stringify_metainfo(tensor);

    ss << content_indent << "{ ";
    for (std::size_t idx = 0; idx < tensor.size(); ++idx) ss << tensor[idx] << (idx + 1 < tensor.size() ? ", " : "");
    ss << " }\n";

    return ss.str();
}

template <class T, Type type, Ownership ownership, Checking checking, Layout layout>
[[nodiscard]] std::string
as_matrix(const GenericTensor<T, Dimension::MATRIX, type, ownership, checking, layout>& tensor) {
    if (tensor.rows() > max_displayed_rows || tensor.cols() > max_displayed_cols) return _as_too_large(tensor);

    // Take care of sparsity using 'fill-ctor + for_each()' - if present, missing elements will just stay "default"
    GenericTensor<std::string, Dimension::MATRIX, Type::DENSE, Ownership::CONTAINER, Checking::NONE, Layout::RC>
        strings(tensor.rows(), tensor.cols(), "-");
    tensor.for_each([&](const T& elem, std::size_t i, std::size_t j) { strings(i, j) = _ss_stringify(elem); });

    // Get appropriate widths for each column - we want matrix to format nicely
    std::vector<std::size_t> column_widths(strings.cols());
    for (std::size_t i = 0; i < strings.rows(); ++i)
        for (std::size_t j = 0; j < strings.cols(); ++j)
            column_widths[j] = std::max(column_widths[j], strings(i, j).size());

    // Output the formatted result
    std::ostringstream ss;
    ss << _stringify_metainfo(tensor);

    for (std::size_t i = 0; i < strings.rows(); ++i) {
        ss << content_indent << "[ ";
        for (std::size_t j = 0; j < strings.cols(); ++j)
            ss << std::setw(column_widths[j]) << strings(i, j) << (j + 1 < strings.cols() ? " " : "");
        ss << " ]\n";
    }

    return ss.str();
}

template <class T, Type type, Ownership ownership, Checking checking, Layout layout>
[[nodiscard]] std::string
as_dictionary(const GenericTensor<T, Dimension::MATRIX, type, ownership, checking, layout>& tensor) {
    if (tensor.size() > max_displayed_flat_size) return _as_too_large(tensor);

    std::ostringstream ss;
    ss << _stringify_metainfo(tensor);

    tensor.for_each([&](const T& elem, std::size_t i, std::size_t j) {
        ss << content_indent << "(" << i << ", " << j << ") = " << _ss_stringify(elem) << "\n";
    });

    return ss.str();
}

// - Export formats -

template <class T, Type type, Ownership ownership, Checking checking, Layout layout>
[[nodiscard]] std::string
as_raw_text(const GenericTensor<T, Dimension::MATRIX, type, ownership, checking, layout>& tensor) {
    // Take care of sparsity using 'fill-ctor + for_each()' - if present, missing elements will just stay "default"
    GenericTensor<std::string, Dimension::MATRIX, Type::DENSE, Ownership::CONTAINER, Checking::NONE, Layout::RC>
        strings(tensor.rows(), tensor.cols(), _ss_stringify(T()));
    tensor.for_each([&](const T& elem, std::size_t i, std::size_t j) { strings(i, j) = _ss_stringify(elem); });

    // Output the formatted result
    std::ostringstream ss;

    for (std::size_t i = 0; i < strings.rows(); ++i) {
        for (std::size_t j = 0; j < strings.cols(); ++j) ss << strings(i, j) << (j + 1 < strings.cols() ? " " : "");
        ss << " \n";
    }
    ss << "\n";

    return ss.str();
}

template <class T, Type type, Ownership ownership, Checking checking, Layout layout>
[[nodiscard]] std::string
as_json_array(const GenericTensor<T, Dimension::MATRIX, type, ownership, checking, layout>& tensor) {
    // Take care of sparsity using 'fill-ctor + for_each()' - if present, missing elements will just stay "default"
    GenericTensor<std::string, Dimension::MATRIX, Type::DENSE, Ownership::CONTAINER, Checking::NONE, Layout::RC>
        strings(tensor.rows(), tensor.cols(), _ss_stringify(T()));
    tensor.for_each([&](const T& elem, std::size_t i, std::size_t j) { strings(i, j) = _ss_stringify(elem); });

    // Get appropriate widths for each column - we want matrix to format nicely
    std::vector<std::size_t> column_widths(strings.cols());
    for (std::size_t i = 0; i < strings.rows(); ++i)
        for (std::size_t j = 0; j < strings.cols(); ++j)
            column_widths[j] = std::max(column_widths[j], strings(i, j).size());

    // Output the formatted result
    std::ostringstream ss;

    ss << "[\n";
    for (std::size_t i = 0; i < strings.rows(); ++i) {
        ss << "  [ ";
        for (std::size_t j = 0; j < strings.cols(); ++j)
            ss << std::setw(column_widths[j]) << strings(i, j) << (j + 1 < strings.cols() ? ", " : "");
        ss << " ]" << (i + 1 < strings.rows() ? "," : "") << " \n";
    }
    ss << "]\n";

    return ss.str();
}

} // namespace format

// Clear out internal macros
#undef _utl_define_operator_support_type_trait
#undef _utl_define_unary_operator_support_type_trait
#undef _utl_template_arg_defs
#undef _utl_template_arg_vals
#undef _utl_require
#undef _utl_reqs

} // namespace utl::mvl

#endif
#endif // module utl::mvl
