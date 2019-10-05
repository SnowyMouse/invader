# How to contribute to Invader
Thank you for your interest! There are a number of ways you can help this project:
- Testing (we need more testers!)
- Submitting [issues](https://github.com/Kavawuvi/Invader/issues)
- Submitting [pull requests](https://github.com/Kavawuvi/Invader/pulls)

## Testing Invader
There are a few ways to obtain builds of Invader:
- Compiling from source (recommended) - Follow the [readme](https://github.com/Kavawuvi/Invader#windows) instructions.
If you're on Arch Linux, you can use the package on the [AUR](https://aur.archlinux.org/packages/invader-git/) for
this, too.
- [Nightly builds](https://invader.opencarnage.net/builds/nightly/download-latest.html) (Windows only) - These builds
are hosted on Open Carnage. A script is ran every once in a while to update this. It may or may not be up-to-date,
however.

To test Invader, run it and use it for making Halo maps and tags. To make maps, you will need a tag set which you can
get by extracting the [Halo Editing Kit installer](https://opencarnage.net/index.php?/topic/2939-halo-editing-kit/).
You can extract the installer by either running it (requires a Halo Custom Edition install and Windows or Wine) or by
using a program like [7-Zip](https://www.7-zip.org/) or [p7zip](https://sourceforge.net/projects/p7zip/).

You should always test using the latest [Git master branch](https://github.com/Kavawuvi/Invader/tree/master) build when
testing Invader. It is not recommended to test even slightly outdated versions of Invader, regardless of if it is a
release or not.

Releases are used only for versioning and keeping track of changes. They are **not** an indication of stability. So, if
the latest release uses code that is older than the Git master branch, then it is just as out-of-date.

## Text conventions
These are general conventions used for writing comments, issues, documentation, and code to ensure it is understandable
by most people.

- Use American English
    - A specific dialect of English is chosen for consistency in spelling
    - Comments inside of issues and pull requests do not specifically need to use the American English dialect, but
    they should not require a translator for American English speakers to read, either.
- Use ISO dates (YYYY-MM-DD)
    - The ISO date format is universally easy to understand, as it removes ambiguity and puts the date components in a
    logical order.
- Use soft tabs of four spaces in length
- Use commas (`,`) when separating thousands, and use decimal points (`.`) when a decimal point is needed
- Use the metric system when applicable. The only exception to this is when talking about world units, which one world
unit in Halo is equal to 10 feet or 3.048 meters.
- Use IEC base-2 units when referring to a large number of bytes (i.e. KiB, MiB, GiB, TiB, etc.)
    - This is to prevent confusion. Windows uses MB and GB as 2^20 and 2^30 bytes, respectively, while almost
    everything else uses MB and GB as 10^3 and 10^6 bytes, respectively.
- All text files must end with a newline
    - Most text editors do this for you

## Scope guidelines
Issues and pull requests must remain in the scope of Invader.

- Invader only applies to the base Halo Custom Edition game. Code or definitions for functionality provided by a mod
should not be added to Invader.
    - For example, if you want to create a fork of Invader that creates Open Sauce maps, you are free to do so provided
    you follow the terms of Invader's license, but you should not submit an issue or a pull request to add this
    functionality as it goes outside the scope of Invader.
- Invader is cross-platform. Platform-specific code such as the Windows API should be avoided in favor of the C/C++
standard library when possible. If this is unavoidable, `#ifdefs` should be used, and functionality should stay the
same on any platform.

## Issue guidelines
Create your issue in the [issues](https://github.com/Kavawuvi/Invader/issues) page.

For all issues:
- Ensure your issue is not a duplicate of another issue. Check both [open](https://github.com/Kavawuvi/Invader/issues)
and [closed](https://github.com/Kavawuvi/Invader/issues?q=is%3Aissue+is%3Aclosed) issues.
- Ensure your issue is relevant for the latest version of the Invader source code. This is why it is recommended that
you compile Invader from source.

For bug reports:
- If the bug is related to a specific data file, please attach or link a compressed archive of the file.
- If the bug is related to a specific tag or tag set, please attach or link a compressed archive of the tag set.
    - The only exception to this rule is if it can be reproduced using the tag set that comes with the Halo Editing Kit
    without any other tags.
- If the bug is related to a specific map, please attach or link a compressed archive of the map file.
- For archives, using either .7z, .tar.xz, .xz, .tar.gz, .gz, or .zip is recommended. Please avoid using proprietary
formats like .rar for attachments.
- The reason for these requirements is to allow others to reproduce your issue. Merely naming the file would mean that
anyone who wanted to investigate and resolve your issue would have to search for the file on the Internet. Not only
would this take time, but there is no guarantee the find the file you're describing would be found, let alone the
correct version of it.

A simple way to remember this is: If a bug cannot be reproduced, then it isn't a bug and your issue will be closed.

## Source file conventions
The Invader repository uses these conventions. If contributing code to Invader, be sure to follow it. This may be
somewhat lengthy, but it is organized to be easy to follow. Some conventions may have been missed, and if this is the
case, they will be added here even if these conventions were discovered upon a pull request being submitted.

If your code does not follow these conventions and you submit a pull request, then you will be asked to revise your
code before it gets merged.

### Exceptions and externally-added code
By "externally-added" code, it is meant that non-Invader code was added into the project. Code that is trivial to
simply rewrite should just be rewritten rather than copied into Invader.

No matter what, all code in the project must be compatible with the project's license. However, in some cases, the rest
of these conventions do not need to be adhered. This is ONLY done for externally-added code, and this is ONLY done to
minimize maintenance.

These are the conventions for externally-added code:
- The **original** license header of the code must still be present
- The License conventions MUST be followed if either of the following are true:
    - Modifications were made to the file by someone besides the original author of the file.
    - The license header does not mention the file name.
- Modifications should be documented at the top of the file below the `SPDX-License-Identifier: GPL-3.0-only` comment
but above the original license header, including the modification to add the license comment but not the modification
list, itself, of course.

### License conventions
- The first line of every source file must be a single-line comment, `SPDX-License-Identifier: GPL-3.0-only`, unless
comments are not supported for that particular file type (e.g. `.tag_indices`)
- Everything must be contributed under the project's license, the GNU General Public License version 3 **only**
- If using GPLv3-only compatible code (e.g. "GPLv2/GPLv3 or later", MIT, public domain, etc.), indicate at the top
of the file (below the aforementioned single-line comment) that this version of the file is released under GPLv3 only.
This is done for clarity.

### File name conventions
- Files and directories should use all-lowercase `snake_case` names with all nouns being singular. There are some
exceptions to this:
- Markdown filenames and the LICENSE file must be written in all caps
- Generated executables use hyphens instead of underscores
- CMakeLists.txt must be written as such because life is unfair
- File extensions, if present, must be lowercase

### Markdown conventions
- Capitalize the first letter of headings as well as any proper noun
    - Do not use title case (e.g. write "Coding conventions" instead of "Coding Conventions")
- Markdown file lines must be no more than 120 characters in length unless it is part of a table
- Markdown table cells must be spaced so they can also be easily read in a text editor. For example:
    ```markdown
    | Format   | Storage  | Bits/px | Alpha   | Red   | Green | Blue  | Notes                 |
    | -------- | -------- | ------- | ------- | ----- | ----- | ----- | --------------------- |
    | `32-bit` | A8R8G8B8 | 32      | 8-bit   | 8-bit | 8-bit | 8-bit |                       |
    |          | X8R8G8B8 | 32      | (8-bit) | 8-bit | 8-bit | 8-bit | All pixels 100% alpha |
    | `16-bit` | R5G6B5   | 16      |         | 5-bit | 6-bit | 5-bit | All pixels 100% alpha |
    |          | A1R5G5B5 | 16      | 1-bit   | 5-bit | 5-bit | 5-bit |                       |
    |          | A4R4G4B4 | 16      | 4-bit   | 4-bit | 4-bit | 4-bit |                       |
    ```

### Coding conventions
These conventions apply to code, specifically. Many of these are best-practices that you should probably use, anyway,
but this is to maintain consistency nonetheless.

#### Message output conventions
- Messages output in the command line that are not variable in length must have no more than 80 characters per line.

#### Naming conventions
Note: These naming conventions do not need to be followed when interfacing with external libraries
- Use snake_case for variables and function definitions
- Constants and enum definitions must be all uppercase
- Use PascalCase for struct, class, enum, and other type definitions
- Besides counters, variables, function names, and type names should be self-explanatory

#### Comment conventions
- Implementation code that is non-trivial or non-self explanatory should have commenting to indicate what it does
    - A good way to think about this: In 10 years, you might not be here to explain what your code does. Can anyone
    with decent amount of programming experience easily understand your code?
- Each comment line should start with a space. (e.g. `// SPDX-License-Identifier: GPL-3.0-only`, `/* asdf */`,
`# This is a comment in Python, CMake, Bash, etc.`, etc.)
- Using [javadoc](https://en.wikipedia.org/wiki/Javadoc)-style documentation in headers is recommended but not
strictly required at this moment. This can and probably will change in the future.

#### Spacing conventions
- Put spaces after commas (e.g. `int x[] = {1, 2, 3, 4, 5};`)
- Do not put spaces after a left-parenthesis or before right-parenthesis (e.g. `printf("2 + 2 = %i\n", (2 + 2));`)
- If there is no operator between a token and a left parenthesis, do not put a space (e.g. `while(true)`)
- Put spaces before and after operators (e.g. `foo + bar` or `x += 10 + (5 + 6);`)
- When writing blocks for functions, loops, and conditionals, put the curly brace on the same line as the function
signature, loop, or conditional, separated by a single space. For example:
    ```cpp
    constexpr int my_function(int start, int end) noexcept {
        // Initialize to 0
        int x = 0;
        for(int i = start; i < end; i++) {
            x += i;
        }
        return x;
    }
    ```

#### C/C++ conventions
- Static and global variables defined in your code may not be modified
- Directory separators in `#include` statements must use forward slashes (e.g. `#include "tag/compiled_tag.hpp"`)
- Do not use Windows-specific types such as `DWORD` unless specifically using Windows API functions and structs
- Never assume the size of fundamental types like `int`, `short`, `size_t`, etc. Use
[fixed-width integer types](https://en.cppreference.com/w/cpp/types/integer) whenever you need a specific size.

##### Headers
- Avoid defining things in the global namespace
    - Invader-specific code should use the `Invader` namespace
    - HEK-specific definitions used in Invader should use the `Invader::HEK` namespace
    - Do not globally use `using namespace` directives such as `using namespace std;`
- Avoid implementation code in headers unless necessary (such as for templates) or the code is very small
- Use `.h` or `.hpp` depending on the target language
    - `.h` files must work with C code
        - Use `#ifdef __cplusplus` to include C++ code such as `extern "C"` (to prevent name mangling when linking)
        and namespaces when the `.h` file is used in C++ code.
    - `.hpp` files must work with C++ code
- Put HEK specific struct definitions in the relevant `hek` directory to avoid confusion with Invader struct
definitions.
- Header files must use C standards-compliant header guards (`#ifdef`/`#define`/`#endif`, and **not** `#pragma once`)
- Header guard constants must resemble the path of the file relative to the source tree to avoid collisions
    - Use all-uppercase characters
    - Use two underscores to separate directories
    - Use one underscore for dots
    - Here is an example header guard (from `invader/map/tag.hpp`):
        ```cpp
        #ifndef INVADER__MAP__TAG_HPP
        #define INVADER__MAP__TAG_HPP
        // code
        #endif
        ```

##### C conventions
- Use `.c` for C source files
- For null pointers, use `NULL`
- Use `#ifdef __cplusplus` and `extern "C"` in header files that are to be included by C++ source and header files to
avoid linking errors.

##### C++ conventions
- Use `.cpp` for C++ source files
- Do not directly use `malloc`, `calloc`, `free`, `new`, `delete`, or `delete[]` unless absolutely necessary
    - Use smart pointers such as `std::unique_ptr` or `std::vector` as these are less error-prone
    - You can use `std::make_unique` to create a `std::unique_ptr` instead of using `new` in the constructor
- Do not use C-style casting, such as `(int)i`
    - Use `static_cast`, `reinterpret_cast`, `const_cast`, or `dynamic_cast` when necessary
- For null pointers, use `std::nullptr`
- When using the C or C++ standard library, use the `std::` namespace explicitly when possible, such as `std::printf`
or `std::uint32_t`
    - The `using namespace std;` directive may result in collisions at some point, so using it is inadvisable
- Do not use `std::cout` or `std::err` for console output. Instead, use `std::printf` and `eprintf`
(`src/eprintf.hpp`), respectively.
- Use C++17's `std::byte` type for raw memory instead of an integer type like `char` or `std::uint8_t`
