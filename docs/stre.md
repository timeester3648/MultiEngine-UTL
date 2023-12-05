

# utl::stre

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

**str**ing **e**xpansions module contains various string-related utilities, such as: [&lt;type_traits&gt;](https://en.cppreference.com/w/cpp/header/type_traits) expansions, universal version of [std::to_string](https://en.cppreference.com/w/cpp/string/basic_string/to_string), inline [std::stringstream](https://en.cppreference.com/w/cpp/io/basic_stringstream) and etc.

## Definitions
```cpp
// Unary type traits
template<typename Type>
struct is_printable;

template<typename Type>
struct is_iterable_through;

template<typename Type>
struct is_const_iterable_through;

template<typename Type>
struct is_tuple_like;

template<typename Type>
struct is_string;

template<typename Type>
struct is_to_str_convertible;

// Conversion to string
template<typename Type>
std::string to_str(const Type &value);

// Inline stringstream
class InlineStream {
public:
	template<typename Type>
	InlineStream& operator<<(const Type &arg);
	
	operator std::string() const;
}
```

In reality `to_str()` is a number of conditionally enabled templates that expand recursively over any passed `Type` as long as types at the end of recursion satisfy `is_printable`.

## Methods
> ```cpp
> stre::is_printable;
> ```

`is_printable<Type>::value` returns at compile time whether `Type` objects can be printed to `std::ostream` with `operator<<()`.

(see and [<type_traits>](https://en.cppreference.com/w/cpp/header/type_traits) and [UnaryTypeTrait](https://en.cppreference.com/w/cpp/named_req/UnaryTypeTrait))


> ```cpp
> stre::is_printable;
> ```

`is_printable<Type>::value` returns at compile time whether `Type` objects can be printed to `std::ostream` with `operator<<()`.

> ```cpp
> stre::is_iterable_through;
> ```

`is_iterable_through<Type>::value` returns at compile time whether `Type` objects can be iterated through.

Object can be iterated through if it has members `.begin()` and `.end()` that return iterators with applicable `operator++()`. 

> ```cpp
> stre::is_const_iterable_through;
> ```

`is_const_iterable_through<Type>::value` returns at compile time whether `Type` objects can be const-iterated through.

Object can be const-iterated through if it has members `.cbegin()` and `.cend()` that return iterators with applicable `operator++()`. 

> ```cpp
> stre::is_tuple_like;
> ```

`is_tuple_like<Type>::value` returns at compile time whether `Type` objects have a structure similar to [std::tuple](https://en.cppreference.com/w/cpp/utility/tuple).

Objects are considered tuple-like if they have applicable standard functions `std::get<0>()` and `std::tuple_size<Type>::value`.

Standard types that are tuple-like include: [std::tuple](https://en.cppreference.com/w/cpp/utility/tuple), [std::pair](https://en.cppreference.com/w/cpp/utility/pair), [std::array](https://en.cppreference.com/w/cpp/container/array) and [std::ranges::subrange](https://en.cppreference.com/w/cpp/ranges/subrange) (since C++20).

> ```cpp
> stre::is_string;
> ```

`is_string<Type>::value` returns at compile time whether `Type` objects can be decayed to `char*`, `const char*` or `std::string`.

> ```cpp
> stre::is_to_str_convertible;
> ```

`is_to_str_convertible<Type>::value` returns at compile time whether `Type` objects can be converted to string with this module's `to_str()` method.

> ```cpp
> std::string stre::to_str(const Type &value);
> ```

Returns `std::string` representation of the object. Accepts all standard containers and tuples with any level of nesting.

Works with any custom datatypes as long at they adhere to standard C++ container structure.

Implemented through recursive template instantiation, that resolves as long as types at the end of recursion satisfy `is_printable`. Custom [std::ostream](https://en.cppreference.com/w/cpp/io/basic_ostream) `operator<<()` overloads are also considered when resolving  whether type is printable.

Respects custom `operator<<()` overloads and gives them priority in recursion, which means any user-defined format will be displayed as is.

> ```cpp
> stre::InlineStream;
> ```

Use `InlineStream() << ...` to build strings using stream syntax without explicitly creating [std::stringstream](https://en.cppreference.com/w/cpp/io/basic_stringstream) object.

## Example 1 (type traits)

[ [Run this code](GODBOLT_LINK) ]
```cpp
using namespace utl;

std::cout
	<< std::boolalpha
	<< "is_printable< int >                    = " << stre::is_printable<int>::value                    << "\n"
	<< "is_iterable_through< std::list<int> >  = " << stre::is_iterable_through<std::list<int>>::value  << "\n"
	<< "is_tuple_like< std::string >           = " << stre::is_tuple_like<std::string>::value           << "\n"
	<< "is_string< const char* >               = " << stre::is_string<const char*>::value               << "\n"
	<< "is_to_str_convertible< std::set<int> > = " << stre::is_to_str_convertible<std::set<int>>::value << "\n";
```

Output:
```
is_printable< int >                    = true
is_iterable_through< std::list<int> >  = true
is_tuple_like< std::string >           = false
is_string< const char* >               = true
is_to_str_convertible< std::set<int> > = true
```

## Example 2 (conversion to string)

[ [Run this code](GODBOLT_LINK) ]
```cpp
using namespace utl;

// Declare objects
std::tuple<int, double, std::string> tup = { 2, 3.14, "text" };
std::vector<std::vector<int>> vec_of_vecs = { {1, 2, 3}, {4, 5, 6}, {7, 8, 9} };
std::unordered_map<std::string, int> map = { {"key_1", 1}, {"key_2", 2} };
std::tuple<std::vector<bool>, std::vector<std::string>, std::vector<std::pair<int, int>>> tup_of_vecs =
		{ { true, false, true }, { "text_1", "text_2" }, { {4, 5}, {7, 8} } };

// Print objects
std::cout
	<< "to_str(tuple):\n" << stre::to_str(tup) << "\n\n"
	<< "to_str(vector of vectors):\n" << stre::to_str(vec_of_vecs) << "\n\n"
	<< "to_str(unordered_map):\n" << stre::to_str(map) << "\n\n"
	<< "to_str(tuple of vectors with bools, strings and pairs):\n" << stre::to_str(tup_of_vecs) << "\n";
```

Output:
```
to_str(tuple):
< 2, 3.14, text >

to_str(vector of vectors):
[ [ 1, 2, 3 ], [ 4, 5, 6 ], [ 7, 8, 9 ] ]

to_str(unordered_map):
[ < key_1, 1 >, < key_2, 2 > ]

to_str(tuple of vectors with bools, strings and pairs):
< [ 1, 0, 1 ], [ text_1, text_2 ], [ < 4, 5 >, < 7, 8 > ] >
```

## Example 3 (inline stringstream)

[ [Run this code](GODBOLT_LINK) ]
```cpp
using namespace utl;

std::string str = stre::InlineStream() << "Value " << 3.14 << " is smaller than " << 6.28;
std::cout << str << "\n";
```

Output:
```
Value 3.14 is smaller than 6.28
```