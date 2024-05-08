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

// Utils
std::string repeat_symbol(char symbol, size_t repeats);
std::string repeat_string(std::string_view str, size_t repeats);
std::string pad_with_zeroes(IntegerType number, std::streamsize total_size = 10);
```

> [!NOTE]
> In reality `to_str()` is a number of [SFINAE-resolved](https://en.cppreference.com/w/cpp/language/sfinae) templates that expand recursively over any passed `Type` as long as types at the end of recursion satisfy `is_printable`.

## Methods

> ```cpp
> stre::is_printable;
> ```

`is_printable<Type>::value` returns at compile time whether `Type` objects can be printed to `std::ostream` with `operator<<()`.

(see and [<type_traits>](https://en.cppreference.com/w/cpp/header/type_traits) and [UnaryTypeTrait](https://en.cppreference.com/w/cpp/named_req/UnaryTypeTrait))

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

`is_string<Type>::value` returns at compile time whether `Type` objects can be converted to `std::string_view` (which is true for all usual string types). Strings denoted by `char16_t`, `char32_t` and `wchar_t` aren't considered "regular strings" in the context of this trait.

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

> ```cpp
> std::string repeat_symbol(char symbol, size_t repeats);
> std::string repeat_string(std::string_view str, size_t repeats);
> ```

Repeats given character or string a given number of times and returns as a string.

> ```cpp
> std::string pad_with_zeroes(IntegerType number, std::streamsize total_size = 10);
> ```

Pads given integer with zeroes until total length reaches `total_size`. Usefull for numbering files/data entries in a way which makes them uniform and lexicographically sortable.

## Example 1 (type traits)

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGe1wAyeAyYAHI%2BAEaYxCAArKQADqgKhE4MHt6%2BekkpjgJBIeEsUTHxdpgOaUIETMQEGT5%2BXLaY9nkM1bUEBWGR0XG2NXUNWc0KQ93BvcX9sQCUtqhexMjsHOYAzMHI3lgA1CYbbggEBAkKIAD0l8RMAO4AdMCECF4RXkorsowED2gslwAIixCMQ8BZUMB0IZUAA3S4JYioIgEACeCWCwAA%2Bl5HLQFJcWExxtFLgolitMAikUQcXiFA8EAkEodsCYNABBdlcsxbBg7Lz7Q5uJQEVnczbbXaYA5HWh4cbiznc4IEPZE4IQOYHADsVk5e0New%2BmL2zDYCgSTFWxoItEO%2Bq5BqN43QIBQSzFzqNRuFwr2rvdEVQnjECQQTG5Pp9fqOBzMZgVWMRqqYEXo/tVsuw0dzecNh0B8bMsrc/vGxHYICTKcEaYzR1VrPdsLEXhl%2Bc7BaO/vMZhMsTcDD7UbzsbcxaThGi9cwWIICCRXmACHLBDdIHlisbglZ2e7Rb7pbXlfdU4IM/Tc4XS5XwsDm4VYp3z7ZG2wLbbHfHxYHQ5H3rRj%2BfZJgQXgJPQWLygA1pga4bhWpp7l2hbFsecYVlWoHgZBMFwUcD6IUYzYgK23gdp2wEJn%2Bw7UYBMY9nGIEKFiRHAP6aAMOMezIBGxAAFT7l2DGHgm6ETphZ4sWxwqcdxvG1PxJFke2wkMWWTHUYOtH9vRvqMROzHzqgrEEMQWKcbC0SOFe8HuqKwpNu%2B%2B6oUeP6SdWLG0hWFkCFZdR4LZBHrvZmDPm4Tlvh%2BpFfuJv7aSOGyOqOhqVmBxAMHsGgOhKOqAtyHALLQnCxLwfgcFopCoJwZaWNYAYUjamw8KQBCaIVCzQSAOpcA8PU6gAHBsUgDWYABsACcXAaPExUcJIZXtVVnC8BcGite1CxwLAMCIB6LAYvQZAUBA/yHf0OyGMAXBjRo600LQF7EBcEAREtETBLUqKcC1H3MMQqIAPIRNoFRtdwvD/GwgiAwwtDfRVvBYO8wBuGI%2BI/UjmBEkY4iI6Q%2BCVpUVkXPjmCqBUuJrJVqqtEt8oRLcAMeFgS1mXgLCYwsVAGMACgAGp4JgdyAwkjCYzIggiGI7BSJL8hKGoS26M0BhGCg1jWPogUXJACyoAk7SkwAtK6hamHVlhcDqezG4DZi8HC0Rglgutai0bRpC4DDuJ4jT%2BD7PRFCU2TJKkAgjE0iRh%2B0Qd9DEYytGDVQTJHejlJUAidHUcczAngxdGnYwTLnIdcAs5LLKsEhFSVi349VHB7KoA1jcbY2SDxavAHsN0PBo/d7BAuCECQ8bDXMvDg1ocwLAgmBMFgMTu3NC2kJzsTreVlWN6tIDrdPhWkNte2egkuLkJQZ10NEoSsGsLdtx3XdXb3Y39/3vCYPgRAu3o/BS1EOIOWACFYqHUPjFWpA7i3ASFzfQddSDb0dpwQGuJz5qlQFQZurd26d0ukYN%2BH8NBDw8AdG%2BxBx7lynptTqIBJDv0mpIMwXBW4zQmj1G6CD5q8A3lvJau9bD7w2ojWepAupmAGg8SRLCxoDViNbDYGgBoTQYdwjY9cd4rRETPWuHAHZIIEdow%2BYj/IpGcJIIAA) ]
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

[ [Run this code](https://godbolt.org/#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGe1wAyeAyYAHI%2BAEaYxCAAbKQADqgKhE4MHt6%2BekkpjgJBIeEsUTHxdpgOaUIETMQEGT5%2BXLaY9nkM1bUEBWGR0XG2NXUNWc0KQ93BvcX9sQCUtqhexMjsHOYAzMHI3lgA1CYbbggEBAkKIAD0l8RMAO4AdMCECF4RXkorsowED2gslwAIixCMQ8BZUMB0IZUAA3S4JYioIgEACeCWCwAA%2Bl5HLQFJcWExxtFLgolitMAikUQcXiFA8EAkEodsCYNABBdlcsxbBg7Lz7Q5uWEVIjEVnczbbXaYA5HAheBL0SWc6X82XytxeBgkLDETDoLFElkbNlqznBAh7InBCBzA4AdisnL2br2H0xe2YbAUCSYqw9BFohxdXNd7uue0BFQMBr2qAi2jFCm57r243QIBAiuVmGFVtIe3QSwi9CLmez4zBRlZe1z8sBTosezMRY2DykRfMZgImFUBB7TsBobT7srIFFDhIwonU/FBcEkrNeynWNQVCxU4UjebzearfbJkdgO7zukewArEXYsfT83HUWABxFgCcd%2BHo4jbonOr10UNY0mFNNwJ2rTEiytOsTV3Y8WzgnsAGtMFRLEuB7It0JPM8rDMMxkNQswMNbD87y/Dl0wnXMVSOOcxRnI4IlQTxWQrAgs0neiJVo9iqwIGtgFYjNeM46duNAkT/TwcTCz2KCzWXbB6yVddN23Rsx3TLS9zg%2BtiC8TAiyoMQlCLfiDOHHCDjwvsBzQ4ie1sggsSIvDLJ088i0vO8cMfPYn1Ik9Pw2MNNL2KNlBra1E2TBxU2/YSOLQXEwvTYVhWs3tUCxasIGozA5hAExLzcBgh3So5hINbNaVy3MHQqtxMuK0qWrKvDUvdRrMtq/iIHnEgEyoVcuIUQq2vKo4Murdgc2y3K1w3LcKjGrUMp7CaSvaoiErSqbKsc%2Ba%2Br/Yh9UAk1xq2ya3Gm/jZt64gIAutaDo6q6ro63auv2prDpyvr8qGkaxJ3O4Xj2JjPAUNiBJ3Qx0D2KTiDGoqPrMF6mpmmqjse3NVOW5BVu6ja0fIsKDUVYgGD2DQyZPbkOAWWhOEvXg/A4LRSFQTgbssawMwpQNNh4UgCE0RmFkQkBHS4B4ZcdJ8NikJ8zFiV8uA0a9mY4SQ2fFrnOF4C4NFF8WFjgWAYEQFBUBYDF6DICgIH%2Be3%2Bh2QxgC4WINBNmhaD7ZHKAifWImCWpUU4EXQ%2BYYhUQAeSTMVI94f42EEOOGFoCOOd4LB3mANwxHxZPSCwIkjHEHPS%2BksU8FFC4q/7CpcTWTmrVafXaDwCJbljjwsH1/i8BYZOFioAxgAUAA1PBMDuOOEkYEv%2BEEEQxHYLsV/kJQ1H13RmgMIwUGsax9G7i5IAWVAEnaBuAFpM0OQFTD5ywuEdPY77jsxeDhaIwSwBfe0LQ2hpBcAwdwnhGj%2BAgT0IoJRsjJFSAIEYTREhIPaHAvoMQxitFrgITowwoGjBAfgjoEwsEzBwYMLoqC9DjC6JQhBXAFjkmWKsCQTMWZ6yrtzDgexVBPliHfWIkg9juyMHsL2DwNAyL2BAXAhBBrCzmLwMWOc5gLAQJgJg%2BpKBcJ1rwEel4Tbs05nwo2IATbqK0ObK2EAkBLDOLicglAXZ0GiKEVgaxBHCNEeIw%2BwApGxBkTI3gholEAL0FvNe4hN6yEUCodQVd96kDuLcBIo99DcNIGY3%2BnA464gSLiIGviRFiIkUE6RsiNDyI8HbDxxBrJK1UabDRksQCSBCWrSQZguBCM1q%2BGWXtsmGNIMY0x%2BsLG2CsW02xHSzBPgeIsvpsQnyXnfhsDQT5XxdNGRsHh5jDZzIlqMn%2BuSpnHJsac0UyMwGSCAA) ]
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

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:1,endLineNumber:5,positionColumn:1,positionLineNumber:5,selectionStartColumn:1,selectionStartLineNumber:5,startColumn:1,startLineNumber:5),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A%0A++++std::string+str+%3D+stre::InlineStream()+%3C%3C+%22Value+%22+%3C%3C+3.14+%3C%3C+%22+is+smaller+than+%22+%3C%3C+6.28%3B%0A++++std::cout+%3C%3C+str+%3C%3C+%22%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
using namespace utl;

std::string str = stre::InlineStream() << "Value " << 3.14 << " is smaller than " << 6.28;
std::cout << str << "\n";
```

Output:
```
Value 3.14 is smaller than 6.28
```

## Example 4 (other utilities)

[ [Run this code](https://godbolt.org/#g:!((g:!((g:!((h:codeEditor,i:(filename:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,selection:(endColumn:77,endLineNumber:9,positionColumn:77,positionLineNumber:9,selectionStartColumn:77,selectionStartLineNumber:9,startColumn:77,startLineNumber:9),source:'%23include+%3Chttps://raw.githubusercontent.com/DmitriBogdanov/prototyping_utils/master/source/proto_utils.hpp%3E%0A%0Aint+main()+%7B%0A++++using+namespace+utl%3B%0A%0A++++std::cout%0A%09%09%3C%3C+%22repeat_symbol(!'h!',++7)+%3D+%22+++%3C%3C+stre::repeat_symbol(!'h!',+++7)+%3C%3C+%22%5Cn%22%0A%09%09%3C%3C+%22repeat_string(%5C%22xo%5C%22,+5)+%3D+%22+%3C%3C+stre::repeat_string(%22xo-%22,+5)+%3C%3C+%22%5Cn%22%0A%09%09%3C%3C+%22pad_with_zeroes(15)+%3D+%22++++++%3C%3C+stre::pad_with_zeroes(15)+++++%3C%3C+%22%5Cn%22%3B%0A%0A++++return+0%3B%0A%7D%0A'),l:'5',n:'0',o:'C%2B%2B+source+%231',t:'0')),k:71.71783148269105,l:'4',n:'0',o:'',s:0,t:'0'),(g:!((g:!((h:compiler,i:(compiler:clang1600,filters:(b:'0',binary:'1',binaryObject:'1',commentOnly:'0',debugCalls:'1',demangle:'0',directives:'0',execute:'0',intel:'0',libraryCode:'0',trim:'1',verboseDemangling:'0'),flagsViewOpen:'1',fontScale:14,fontUsePx:'0',j:1,lang:c%2B%2B,libs:!(),options:'-std%3Dc%2B%2B17+-O2',overrides:!(),selection:(endColumn:1,endLineNumber:1,positionColumn:1,positionLineNumber:1,selectionStartColumn:1,selectionStartLineNumber:1,startColumn:1,startLineNumber:1),source:1),l:'5',n:'0',o:'+x86-64+clang+16.0.0+(Editor+%231)',t:'0')),header:(),l:'4',m:50,n:'0',o:'',s:0,t:'0'),(g:!((h:output,i:(compilerName:'x86-64+clang+16.0.0',editorid:1,fontScale:14,fontUsePx:'0',j:1,wrap:'1'),l:'5',n:'0',o:'Output+of+x86-64+clang+16.0.0+(Compiler+%231)',t:'0')),k:46.69421860597116,l:'4',m:50,n:'0',o:'',s:0,t:'0')),k:28.282168517308946,l:'3',n:'0',o:'',t:'0')),l:'2',n:'0',o:'',t:'0')),version:4) ]
```cpp
using namespace utl;

std::cout
	<< "repeat_symbol('h',  7) = "   << stre::repeat_symbol('h',   7) << "\n"
	<< "repeat_string(\"xo\", 5) = " << stre::repeat_string("xo-", 5) << "\n"
	<< "pad_with_zeroes(15) = "      << stre::pad_with_zeroes(15)     << "\n";
```

Output:
```
repeat_symbol('h',  7) = hhhhhhh
repeat_string("xo", 5) = xo-xo-xo-xo-xo-
pad_with_zeroes(15) = 0000000015
```