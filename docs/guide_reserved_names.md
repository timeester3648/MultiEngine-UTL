# Names reserved by UTL implementation

[<- back to README.md](https://github.com/DmitriBogdanov/prototyping_utils/tree/master)

Due to the header-only nature of the library, all internal identifiers can be accessed from outside.

Identifiers that are not a part of the public interface are considered **implementation identifiers**. All such identifiers follow a standardized naming convention that aims to reduce the probability of accidental name collisions.

## Reserved Local Identifiers

All identifiers residing inside a module namespace and prefixed with `_`:

```cpp
utl::<module_name>::_<local_identifier_name>
```

## Reserved Global Identifiers

All global identifiers prefixed with `UTL_`:

```cpp
UTL_<global_identifier_name>
```

