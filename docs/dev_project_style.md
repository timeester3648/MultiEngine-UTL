# Project style

This file contains specifications that ensure a standardized style of source files, documentation, commits and etc.

## Source files

### Module template

```cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/prototyping_utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::XXXXXXXXXXXX
// Documentation: https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/module_XXXXXXXXXXXX.md
// Source repo:   https://github.com/DmitriBogdanov/prototyping_utils
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMODULE_XXXXXXXXXXXX)
#ifndef UTLHEADERGUARD_XXXXXXXXXXXX
#define UTLHEADERGUARD_XXXXXXXXXXXX

// _______________________ INCLUDES _______________________

// NOTE: STD INCLUDES

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS

// ____________________ IMPLEMENTATION ____________________

namespace utl::XXXXXXXXXXXX {

// NOTE: IMPL

} // namespace utl::XXXXXXXXXXXX

#endif
#endif // module utl::XXXXXXXXXXXX

```

Replace `XXXXXXXXXXXX` with **module name**.

### Macro-Module template

```cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/prototyping_utils ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Macro-Module:  UTL_XXXXXXXXXXXX
// Documentation: https://github.com/DmitriBogdanov/prototyping_utils/blob/master/docs/MACRO_XXXXXXXXXXXX.md
// Source repo:   https://github.com/DmitriBogdanov/prototyping_utils
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#if !defined(UTL_PICK_MODULES) || defined(UTLMACRO_XXXXXXXXXXXX)
#ifndef UTLHEADERGUARD_XXXXXXXXXXXX
#define UTLHEADERGUARD_XXXXXXXXXXXX

// _______________________ INCLUDES _______________________

// NOTE: STD INCLUDES

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS

// ____________________ IMPLEMENTATION ____________________

// NOTE: IMPL

#endif
#endif // macro-module UTL_XXXXXXXXXXXX

```

Replace `XXXXXXXXXXXX` with **macro-module name**.

### Module tests template

```cpp
// _______________ TEST FRAMEWORK & MODULE  _______________

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "thirdparty/doctest.h"

#include "module_XXXXXXXXXXXX.hpp"

// _______________________ INCLUDES _______________________

// NOTE: STD INCLUDES

// ____________________ DEVELOPER DOCS ____________________

// NOTE: DOCS

// ____________________ IMPLEMENTATION ____________________

// NOTE: IMPL

```

Replace `XXXXXXXXXXXX` with **module name**.

### In-code headers

#### Header 1

```cpp

// ================
// --- Header 1 ---
// ================

```

#### Header 2

```cpp

// --- Header 2 ---
// ----------------

```

#### Header 3

```cpp

// - Header 3 -
```

## Documentation

TODO:

## Commit Style
### Commit types

| Type | Description |
| - | - |
| `feat` | New features |
| `test` | New unit tests |
| `fix` | Bugfixes |
| `refactor` | Code refactors |
| `docs` | Documentation changes |
| `build` | Build script changes |
| `chore` | Typo fixes, file renames and etc |

### Commit message format

```
<type>(<scope>): <summary>.
// repeat for all changes according to commit types
// <scope> can be ommited for some minor changes
// Use <scope> = GLOBAL to signify large sweeping changes that affect the whole project
```

### Commit message example

```
feat(mvl): Mostly implemented sparse and dense blocking operations.

chore(): Typo fixes in comments & documentation.

docs(mvl): Fixes some incorrect definitions and wonky SVG rendering.

docs(): Improved table with used tools & libraries.
```