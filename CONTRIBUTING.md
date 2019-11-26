# How to contribute to Invader
Thank you for your interest! There are a number of ways you can help this
project:

- Testing (we need more testers!)
- Submitting [issues]
- Submitting [pull requests]

# Testing Invader
To test Invader, run it and use it for making Halo maps and tags. To make maps,
you will need a tag set which you can get by extracting the [Halo Editing Kit]
installer.

You can extract the installer by either running it (requires a Halo Custom
Edition install and Windows or Wine) or by using a program like [7-Zip] or
[p7zip].

You should always test using the latest [Git master branch] build when
testing Invader. It is not recommended to test even slightly outdated versions
of Invader, regardless of if it is a release or not.

## Stable releases
Releases are used only for versioning and keeping track of changes. They are
**not** an indication of stability. So, if the latest release uses code that is
older than the Git master branch, then it is just as out-of-date and,
therefore, less "stable" as a result.

# Conventions
These conventions apply only for contributing to the official Invader
repository. Forks or other derivatives of Invader may use completely different
conventions.

These conventions are subject to change.

## Text conventions
These are general conventions used for writing comments, issues, documentation,
and code to ensure it is understandable by most people.

- Use American English
    - A specific dialect of English is chosen for consistency in spelling
    - Comments inside of issues and pull requests do not specifically need to
      use the American English dialect, but they should not require a
      translator for American English speakers to read, either.
- Use ISO dates (YYYY-MM-DD). This date format is universally easy to
  understand as there is no ambiguity, and the components of the date is
  ordered logically from most significant (year) to least significant (day).
- Use soft tabs of four spaces in length
- Use commas (`,`) when separating thousands, and use decimal points (`.`) when
  a decimal point is needed
- Use the metric system when applicable. The only exception to this is when
  talking about world units, which one world unit in Halo is equal to 10 feet
  or 3.048 meters.
- Use IEC base-2 units when referring to a large number of bytes (i.e. KiB,
  MiB, GiB, TiB, etc.)
    - This is to prevent confusion. Windows uses MB and GB as 2^20 and 2^30
      bytes, respectively, while almost everything else uses MB and GB as 10^3
      and 10^6 bytes, respectively.
- All text files must end with a newline. Most modern text editors will do this
  for you.

## Issue conventions
Create your issue in the [issues] page.

For all new issues:
- Ensure your issue is not a duplicate of another issue. Check both open and
  [closed] issues.
- Ensure your issue is relevant for the latest version of the Invader source
  code. This is why it is recommended that you compile Invader from source.
- Ensure your issue is not out of scope (see the [scope conventions])

For new issues, it is recommended to prefix the title with **one** of these:
- `[New feature]` - New feature for multiple programs or one that doesn't yet
  exist
- `[invader-XXXXXX]` - Applies to a specific Invader program or a part of the
  Invader library that is typically only used by a specific Invader program
- `[invader]` - This is an issue for the Invader library

If you don't do this, a moderator will try to do it for you, instead.

For bug reports:
- If the bug is related to a specific data file, please attach or link a
  compressed archive of the file.
- If the bug is related to a specific tag or tag set, please attach or link a
  compressed archive of the tag set.
    - The only exception to this rule is if it can be reproduced using the tag
      set that comes with the Halo Editing Kit without any other tags.
- If the bug is related to a specific map, please attach or link a compressed
  archive of the map file.
- For archives, using either .7z, .tar.xz, .xz, .tar.gz, .gz, or .zip is
  recommended. Please avoid using proprietary formats like .rar for
  attachments.
- The reason for these requirements is to allow others to reproduce your issue.
  Merely naming the file would mean that anyone who wanted to investigate and
  resolve your issue would have to search for the file on the Internet. Not
  only would this take time, but there is no guarantee the find the file you're
  describing would be found, let alone the correct version of it.

A simple way to remember this is: If a bug cannot be reproduced, then it isn't
a bug and your issue will be closed.

## Scope conventions
Issues and pull requests must remain in the scope of Invader. Here are some
things to keep in mind:

- Invader targets the base Halo Combat Evolved game as released on PC.
- Invader is cross-platform and does not target any single operating system.
- Invader does not (usually) care about the limitations of the original Halo
  Editing Kit.

Remember, these conventions only apply to contributions towards the main
Invader repository, not forks. This means that you are allowed to create a fork
of Invader that targets a specific mod of the game (e.g. Chimera, Open Sauce,
etc.), provided you follow the license of Invader and, if necessary, the mod
you are targeting. However, code or definitions for functionality provided by
these mods will not be accepted upstream.

Also, if you contribute to Invader and write C/C++ code, remember to use
functions and data types standard to C/C++. Do not use platform-specific types
or functions such as the Windows API's `DWORD` or `fopen_s` when writing any
platform-independent code. There may be times where such platform-specific code
is necessary. If so, ensure you use `#ifdef`s so the code is only compiled when
that platform is being targeted.

Lastly, it is well known that the Halo Editing Kit imposes limitations on
structures. For example:
- tool.exe limits the maximum size of a bitmap to 16 MiB
- tool.exe limits the maximum number of structs in most tags
- tool.exe limits the file size of maps to 128 MiB in multiplayer and 384 MiB
  in singleplayer
- and so on...

As long as it is otherwise valid data, then Invader should not prevent the user
from creating assets, even if these assets may not work with the Halo Editing
Kit. In fact, Invader should not be expected to warn the user if such assets
will not work with the original Halo Editing Kit. Usage of the original Halo
Editing Kit should be discouraged whenever possible.

## Source file conventions
The Invader repository uses these conventions. If contributing code to Invader,
be sure to follow it. This may be somewhat lengthy, but it is organized to be
easy to follow. Some conventions may have been missed, and if this is the case,
they will be added here even if these conventions were discovered upon a pull
request being submitted.

### Exceptions and externally-added code
By "externally-added" code, it is meant that non-Invader code was added into
the project. Code that is trivial to simply rewrite should just be rewritten
rather than copied into Invader.

No matter what, all code in the project must be compatible with the project's
license. However, in some cases, the rest of these conventions do not need to
be adhered. This is ONLY done for externally-added code, and this is ONLY done
to minimize maintenance.

These are the conventions for externally-added code:
- The **original** license header of the code must still be present
- The License conventions MUST be followed if either of the following are true:
    - Modifications were made to the file by someone besides the original
      author of the file.
    - The license header does not mention the file name.
- Modifications should be documented at the top of the file below the
  `SPDX-License-Identifier: GPL-3.0-only` comment but above the original
  license header, including the modification to add the license comment but not
  the modification list, itself, of course.

### License conventions
These apply to license headers and licensing.

- The first line of every source file must be a single-line comment,
  `SPDX-License-Identifier: GPL-3.0-only`, unless comments are not supported
  for that particular file type (e.g. `.tag_indices`)
- Everything must be contributed under the project's license, the GNU General
  Public License version 3 **only**
- If using GPLv3-only compatible code (e.g. "GPLv2/GPLv3 or later", MIT, public
  domain, etc.), indicate at the top of the file (below the aforementioned
  single-line comment) that this version of the file is released under GPLv3
  only. This is done for clarity.

### File name conventions
This applies to how files and directories are named.

- Files and directories should use all-lowercase `snake_case` names with all
  nouns being singular. There are some exceptions to this:
- Markdown filenames and the LICENSE file must be written in all caps
- Generated executables use hyphens instead of underscores
- CMakeLists.txt must be written as such because life is unfair
- File extensions, if present, must be lowercase

### Markdown conventions
This applies to how markdown files are created and edited.

- Capitalize the first letter of headings as well as any proper noun
    - Do not use title case (e.g. write "Coding conventions" instead of "Coding
      Conventions")
- Markdown file lines must be no more than 80 characters in length unless it
  is part of code, or it is a URL on the bottom of the page.
- Markdown table cells must be spaced so they can also be easily read in a text
  editor. For example:
  ```markdown
  Format   | Storage  | Bits/px | Alpha   | Red   | Green | Blue  | Notes                
  -------- | -------- | ------- | ------- | ----- | ----- | ----- | ----------
  `32-bit` | A8R8G8B8 | 32      | 8-bit   | 8-bit | 8-bit | 8-bit |                      
           | X8R8G8B8 | 32      | (8-bit) | 8-bit | 8-bit | 8-bit | 100% alpha
  `16-bit` | R5G6B5   | 16      |         | 5-bit | 6-bit | 5-bit | 100% alpha
           | A1R5G5B5 | 16      | 1-bit   | 5-bit | 5-bit | 5-bit |                      
           | A4R4G4B4 | 16      | 4-bit   | 4-bit | 4-bit | 4-bit |                      
  ```

### Coding conventions
These conventions apply to code, specifically. Some of these are best-practices
that you should probably use, anyway, but this is to maintain consistency
nonetheless.

#### Message output conventions
This applies to feedback given to the user by the program.

- Messages output in the command line that are not variable in length must have
  no more than 80 characters per line
- Error messages (including usage) should use standard error (stderr)

#### Naming conventions
These apply to the definitions of variables, functions, types, structs, class,
enums, etc.

- Use snake_case for variable and function definitions
- Constants and enum definitions must be all uppercase
- Use PascalCase for struct, class, enum, and other type definitions
- Besides counters, variables, function names, and type names should be
  self-explanatory

#### Comment conventions
These apply to comments and when to comment.

- Implementation code that is non-trivial or non-self explanatory should have
  commenting to indicate what it does
    - A good way to think about this: In 10 years, you might not be here to
      explain what your code does. Can anyone with decent amount of programming
      experience easily understand your code?
- Using [javadoc]-style documentation in headers is recommended but not
  strictly required at this moment. This can and probably will change in the
  future.

#### C/C++ conventions
These apply to C/C++ code specifically.

- Static and global variables defined in your code may not be modified
    - The reason for this is because it's usually not thread safe, and
      singletons are often a result of poor design
- Directory separators in `#include` statements must use forward slashes (e.g.
  `#include "tag/compiled_tag.hpp"`)
- Use [fixed-width integer types] whenever you need a specific size
  of integer. Never assume the size of fundamental types like `int`, `short`,
  `size_t`, etc.

##### Headers
These apply to C/C++ header files.

- Avoid defining things in the global namespace
    - Invader-specific code should use the `Invader` namespace
    - HEK-specific definitions used in Invader should use the `Invader::HEK`
      namespace
    - Do not globally use `using namespace` directives such as
      `using namespace std;`
- Avoid implementation code in headers unless necessary (such as for templates)
  or the code is very small
- Use `.h` or `.hpp` depending on the target language
    - `.h` files must work with C code
        - Use `#ifdef __cplusplus` to include C++ code such as `extern "C"` (to
          prevent name mangling when linking) and namespaces when the `.h` file
          is used in C++ code.
    - `.hpp` files must work with C++ code
- Put HEK specific struct definitions in the relevant `hek` directory to avoid
  confusion with Invader struct definitions.
- Header files must use C standards-compliant header guards (**not**
  `#pragma once`)
- Header guard constants must resemble the path of the file relative to the
  source tree to avoid collisions
    - Guard constant must begin with with `INVADER__`
    - Use all-uppercase characters
    - Use two underscores to separate directories relative to src, not
      including the src folder itself
    - Use one underscore for dots
    - Here is an example header guard (from `invader/map/tag.hpp`):
        ```cpp
        #ifndef INVADER__MAP__TAG_HPP
        #define INVADER__MAP__TAG_HPP
        // code
        #endif
        ```

##### C conventions
These apply to C code.

- Use `.c` for C source files
- For null pointers, use `NULL`

##### C++ conventions
These apply to C++ code.

- Use `.cpp` for C++ source files
- Do not directly use `malloc`, `calloc`, `free`, `new`, `delete`, or
  `delete[]` unless absolutely necessary
    - Use smart pointers such as `std::unique_ptr` or `std::vector` as these
      are less error-prone
    - You can use `std::make_unique` to create a `std::unique_ptr` instead of
      using `new` in the constructor
- Do not use C-style casting, such as `(int)i`
    - Use the correct casting functions (e.g. `static_cast`,
      `reinterpret_cast`, `const_cast`, etc.) when necessary
- For null pointers, use `nullptr`
- When using the C or C++ standard library, use the `std::` namespace
  explicitly when possible, such as `std::uint32_t`
    - The `using namespace std;` directive may result in collisions at some
      point, so using it is inadvisable
- Do not use `std::cout` or `std::err` for console output. Instead, use
  `oprintf` and `eprintf` (`src/printf.hpp`), respectively.
- Use C++17's `std::byte` type for raw memory instead of an integer type like
  `char` or `std::uint8_t`

[issues]: https://github.com/Kavawuvi/Invader/issues
[pull requests]: https://github.com/Kavawuvi/Invader/pulls
[Halo Editing Kit]: https://opencarnage.net/index.php?/topic/2939-halo-editing-kit/
[7-Zip]: https://www.7-zip.org/
[p7zip]: https://sourceforge.net/projects/p7zip/
[javadoc]: https://en.wikipedia.org/wiki/Javadoc
[closed]: https://github.com/Kavawuvi/Invader/issues?q=is%3Aissue+is%3Aclosed
[git master branch]: https://github.com/Kavawuvi/Invader/tree/master
[fixed-width integer types]: https://en.cppreference.com/w/cpp/types/integer
[scope conventions]: #scope-conventions
