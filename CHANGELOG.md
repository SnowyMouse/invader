# Invader changelog
This is used for recording Invader's changes. This changelog is based on
[Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [Unreleased]
### Added
- Added `--info` to invader-indexer to show information about Invader, similar
  to other Invader programs
- invader-font: Added support for .otf files
- invader-info: Added `-T stub-count`

### Changed
- To prevent people from confusing arguments of different programs, two of the
  short argument letters were changed:
    - invader-bitmap: Changed `-m` to `-M` to avoid confusion with `maps`
    - invader-build: Changed `-R` to `-N` to avoid confusion with `retail`
- invader-build: 32 byte tag strings are now zeroed out before copying
- invader-compress: Names and build strings are zeroed out before copying
- invader-build: The number of stubbed tags is now shown next to the tag count
- invader-info: The number of stubbed tags is now shown next to the tag count

### Fixed
- invader-font: Fixed `-i` being used as both info and font size. It is now
  `-z` as expected.

## [0.16.1] - 2019-11-04
### Added
- invader-info: Added `-T compression-ratio`
- invader-info: Added `-T build`
- invader-info: Added `-T crc32-mismatched`

### Changed
- Using `-h` or `--help` no longer outputs an extra newline
- invader-build: Accepts forward slashes and backslashes in index files
- invader-info: Dirty check is now separate from CRC32 but still checks CRC32
- invader-info: Dirty check now checks if the map has been modified by Refinery
- invader-info: Dirty check now fails (returns "Dirty") if the map is protected

### Fixed
- invader-bitmap: Palettized flag is now properly set
- invader-info: Fixed detecting stubbed tags as protected. However, this will
  not apply to maps built before this change.

## [0.16.0] - 2019-11-04
### Added
- invader-info - Displays an overview of a map file, but also has options:
  compressed, crc32, dirty, engine, map-type, scenario, scenario-path,
  tag-count, tags
- invader-build: Added `-R` to rename scenarios when building

### Changed
- Help menus have been rewritten for most programs
- invader-build: Shows map type when building
- invader-indexer: Changed index format to simply list tag paths with extension
- invader-build: Use new indexer - *NOTE: Previous index files no longer work*

### Removed
- invader-crc: Removed invader-crc in favor of invader-info. Use `-T crc32` or
  `-T dirty` to indicate the map CRC32 and whether the map is dirty or not in a
  scriptable fashion.

## [0.15.2] - 2019-10-29
### Changed
- The zstd library is now static linked (except for libarchive)

### Fixed
- invader-compress: Fixed a bug where decompressing did not work
- invader-dependency: Fixed a bug where reverse and recursive were switched

## [0.15.1] - 2019-10-29
### Changed
- invader-compress: Now uses `-d` instead of `-D` to decompress
- invader-dependency: Now uses `-R` instead of `-r` for reverse dependencies
- invader-dependency: Now uses `-r` instead of `-R` for recursive dependencies

## [0.15.0] - 2019-10-29
### Added
- invader-compress - Compresses cache files using the [Zstandard] algorithm
- invader-resource: Added `--retail` / `-R` which will allow you to build
  retail maps
- invader-build: Copying TagString data now checks for string overflow
- invader-crc: Works with maps compressed with invader-compress
- invader-indexer: Works with maps compressed with invader-compress

### Changed
- invader-build: Bitmaps and sound data are now shown separately next to their
  combined total in the diagnostic output
- invader-build: The number of tags that could be cached/indexed is now shown
- invader-archive: Now handles exceptions instead of calling abort() while
  either compiling or parsing cache files
- C++ header files to include were moved to an include directory
- invader-build: No longer errors when orphaned model nodes exist

### Fixed
- invader-build: Fixed some error messages not being shown using the correct
  directory separators
- invader-build: Fixed indexing tags with retail/demo resources
- invader-archive: Fixed some issues with `-P`
- invader-build: Fixed decals with null references crashing the game

## [0.14.1] - 2019-10-19
### Changed
- invader-build: The diagnostic messages now show tags using external raw data
  when building a retail or demo map
- invader-build: The BSP path is now shown using the system's preferred path
  separator
- invader-dependency: Tag paths are now shown using the system's preferred path
  separator

### Fixed
- invader-bitmap: Fully implemented and fixed 3D textures

## [0.14.0] - 2019-10-18
### Added
- Added a CONTRIBUTING.md file which specifies guidelines for contributing to
  Invader
- invader-archive: Now accepts filesystem paths for the tag if `-P` is passed
- invader-bitmap: Now accepts filesystem paths for the image input if `-P` is
  passed
- invader-build: Now accepts filesystem paths for the scenario tag if `-P` is
  passed
- invader-dependency: Now accepts filesystem paths for the tag if `-P` is
  passed
- invader-font: Now accepts filesystem paths for the the TTF file if `-P` is
  passed
- invader-string: Now accepts filesystem paths for the text file if `-P` is
  passed
- invader-crc: Now prints a warning to stderr if the CRC32 in the cache file
  header is wrong
- invader-crc: Now errors with a useful error message if a resource map is
  loaded with it
- Map parser: Now errors if the internal map name overflows into the build
  string
- invader-build: Halo Demo / Trial maps can now be built (`-g demo`)
- invader-crc: Halo Demo / Trial maps can now be used
- invader-build: --no-indexed-tags was renamed to --no-external-tags
- invader-build: Building retail and demo maps now uses the resource maps
- invader-build: The gain modifier for `sound\sfx\impulse\ting\ting.sound` is
  now changed to 0.2 if building for retail or demo and 1.0 otherwise
- invader-build: Now shows the target engine of the map

### Changed
- Replaced the command line arg parser. The new one was made from scratch and
  uses only functions from the C++ standard library, allowing Invader to
  compile on more systems provided they support C++17.
- Replaced the license header in each source file with a simpler
  `// SPDX-License-Identifier: GPL-3.0-only`. For more information on this
  header, go to https://spdx.org/ids-how
- invader-archive: Now uses stat() to get the modification date of a file
- invader-bitmap: Made detail fade factor closer to tool.exe's output - still
  needs more work but it's remarkably close

### Fixed
- invader-dependency: Fixed an issue where it didn't include a null terminator,
  causing some tags to fail to open
- invader-archive: Fixed an issue where some stock HEK .gbxmodel tags tried
  (and failed) to be archived as .model tags
- invader-bitmap: Fixed an issue where sprites spanning the entire width
  wouldn't be detected
- invader-archive: Fixed an issue where the root tag would have its full system
  path included when using `-s`

## [0.13.0] - 2019-10-04
### Changed
- invader-build: Changed how stubbed tags are created so tag extractors won't
  try to extract them
- invader-bitmap: Errors if the tag contains uppercase characters in its path
- invader-build: Errors if any non-stubbed tags contain uppercase characters in
  their paths
- invader-font: Errors if the tag contains uppercase characters in its path
- invader-string: Errors if the tag contains uppercase characters in its path

### Fixed
- invader-build: Fixed not fixing the render bounding radius if it was less
  than the bounding radius but non-zero.
- invader-build: Fixed not setting the weight value for color change
  permutations in objects.
- invader-bitmap: Fixed detail fade factor so it matches tool.exe's detail fade
  factor more closely if not exactly.
- invader-build: Fixed certain sound permutation file offsets not being
  correctly marked as internal; this should fix
some sounds that sounded fine when built with tool.exe but sounded corrupt when
  built with invader-build

### Removed
- invader-bitmap: Removed --filter-blur and --filter-sharpen. Tags that have
  these values set will still have the filter(s) applied. However, for newer
  tags, you should use an image editor, as you will get similar or better
  results.
- invader-bitmap: Removed --sprite-spacing and used tool.exe's broken
  functionality, instead.
- invader-bitmap: Removed the ability to set sprite budgets below 32 or above
  512.

## [0.12.0] - 2019-09-22
### Changed
- invader-bitmap: Changed --mipmap-blur and --mipmap-sharpen to --filter-blur
  and --filter-sharpen. These now *only* affect the first bitmap even though
  image editors exist that can do these exact things. Oh well.

### Fixed
- invader-bitmap: Fixed the help list line breaks

## [0.11.0] - 2019-09-22
### Added
- invader-bitmap: Added --mipmap-blur (`blur filter size`); this won't affect
  the bitmap unlike what tool.exe does (as Guerilla implies only the mipmaps
  are affected) - if you want to do this to the bitmap, itself, use an image
  editor
- invader-bitmap: Added --mipmap-sharpen (`sharpen amount`); this won't affect
  the bitmap unlike what tool.exe does (as Guerilla implies only the mipmaps
  are affected) - if you want to do this to the bitmap, itself, use an image
  editor

### Changed
- invader-bitmap: Switched to the Xbox P8 palette. This palette is probably
  worse, but the original Halo Editing Kit as well as [Mozzarilla] support it,
  so it's easier to use.
- invader-bitmap: Now stores dithering and mipmapping settings in the tag

### Fixed
- invader-bitmap: Fixed blurring and sharpening values in the tag not being
  retained

## [0.10.1] - 2019-09-17
### Changed
- Modified the project license terms to strictly GPL version 3.

## [0.10.0] - 2019-09-16
### Added
- invader-bitmap: Added --usage (default, bumpmap, and detail) and
  `--bump-height` parameters
- invader-bitmap: Added p8-bump support based on Stubbs the Zombie's palette

### Changed
- invader-bitmap: Spacing now attempts to sort both vertical and horizontal
  to see if sprites will fit in a sprite
sheet
- invader-bitmap: Changed how spacing is stored in the bitmap to effectively
  match how tool.exe calculates its spacing
- invader-bitmap: Double multiply sprites now simply replaces the pixel like
  tool.exe rather than alpha blend into gray
- invader-bitmap: Usage and the p8 compression flag are now preserved
- invader-bitmap: Height maps now generate bump maps similar to tool.exe
- invader-bitmap: --detail-fade replaces --mipmap-fade and is now only usable
  on detail maps.
- invader-bitmap: --detail-fade now approximately matches how tool.exe does
  fade to gray
- invader-bitmap: Dithering is now available for 16-bit and palettized bitmaps
- invader-bitmap: Dithering now takes an argument: `<channels>`. Channels are
  letters (i.e. `argb`).

### Fixed
- invader-bitmap: Fixed some issues with spacing

## [0.9.0] - 2019-09-13
### Changed
- invader-bitmap: Changed detection to first line is blue instead of first
  pixel for determining when to read a whole
image as one bitmap

### Removed
- invader-bitmap: Removed -O (this is now done by default)
- invader-bitmap: Removed the ability to create non 1:1 sprite sheets due to
  them not working well with particles

## [0.8.2] - 2019-09-11
### Added
- A changelog is now used to track changes

### Fixed
- Fixed zero spacing causing hangs

## [0.8.1] - 2019-09-11
### Added
- invader-bitmap: 3D textures are now supported

### Changed
- invader-bitmap: The cyan pixel is now optional in color plates
- invader-bitmap: Alpha is now ignored in color plates
- invader-bitmap: Textures that don't utilize blue backgrounds are now detected
  differently
- invader-bitmap: Sprites no longer require valid color plates
- invader-bitmap: Sprite spacing now affects the maximum number of mipmaps you
  can have with sprites

### Fixed
- invader-bitmap: Fixed certain bitmaps going over the specified budget

## [0.8.0] - 2019-09-11
### Added
- invader-bitmap: Added support for bitmaps with color plate data
- invader-bitmap: Added support for cubemaps, sprites, and interface bitmaps
- invader-bitmap: Added the ability to specify custom spacing for sprites
- invader-bitmap: Added the ability to specify bitmap type in the command-line

### Changed
- invader-bitmap: Non-divisible by 4 bitmaps are no longer compressed
- invader-bitmap: Registration point calculation has been changed to better
  match tool.exe's calculations
- invader-bitmap: Alpha is now ignored when checking if a pixel is blue,
  magenta, or cyan

### Fixed
- invader-build: Fixed an issue with certain sounds not being played correctly,
  such as the "Come on! We've got to get the hell out of here!" dialogue at the
  start of the game

### Removed
- invader-bitmap: Removed being able to specify negative mipmaps to remove
  mipmaps

## [0.7.3] - 2019-09-05
### Fixed
- invader-string: Fixed cutting the last character of every string

## [0.7.2] - 2019-09-05
### Fixed
- invader-string: Fixed doubling the string every line

## [0.7.1] - 2019-09-05
### Fixed
- invader-build: unicode_string_list tags' string data is now compared when
  considering whether to index a tag or not

## [0.7.0] - 2019-09-04
### Added
- invader-string: Generates unicode_string_list and string_list tags

### Fixed
- invader-bitmap: Fixed pixels offset not being set multi-bitmap bitmap tags
- invader-build: Fixed CRC32 being displayed even when quiet

## [0.6.0] - 2019-08-26
### Changed
- invader-build: Some hidden values are now calculated for machines
- invader-build: All multiplayer maps use stock limits for indexed resource,
  while all singleplayer and UI maps can use the extended Invader resource
  limits

## [0.5.0] - 2019-08-24
### Changed
- invader-build: CRC32 can now be specified when building
- invader-build: CRC32 is now calculated in the header when building

### Removed
- invader-crc: CRC32 can no longer be forged using this tool; use invader-build instead

## [0.4.3] - 2019-08-23
### Changed
- invader-build: Encounter firing positions and squad positions are now
  raycasted for determining the BSP for an encounter
- invader-font: The width of the 'X' character is now used for the dot width.

## [0.4.2] - 2019-08-23
### Fixed
- invader-font: Fixed the dot being missing when a font is used as a console
  font
- invader-font: Fixed some invisible characters being rendered by mistake

## [0.4.1] - 2019-08-23
### Changed
- invader-font: now creates directories when outputting a font tag

### Fixed
- invader-indexer: Fixed a missing newline in a help message

## [0.4.0] - 2019-08-22
### Added
- invader-font: Generates font tags from .ttf files in the data folder

## [0.3.2] - 2019-08-22
### Fixed
- invader-build: Fixed only half of the bitmaps and sounds being indexed

## [0.3.1] - 2019-08-22
### Fixed
- invader-build: Fixed incorrect limits being used for ui/multiplayer maps

## [0.3.0] - 2019-08-22
### Changed
- invader-build: Custom maps now have enforced index limits

## [0.2.1] - 2019-08-21
### Changed
- invader-build: Command list BSP locations are now calculated
- invader-build: Some warnings were removed

## [0.2.0] - 2019-08-21
### Added
- invader-archive - Archives all tags needed to build a map
- invader-bitmap - Creates bitmap tags (only supports 2D textures without color
  plate data but also has custom mipmap support)
- invader-build - Builds cache files
- invader-crc - Displays and spoofs CRC32 checksums of cache files; this is
  useful for using modified multiplayer maps
- invader-dependency - Outputs a list of tags that depend on a given tag
- invader-indexer - Outputs the list of tags in a cache file to a text file to
  be used with Invader
- invader-resource - Builds resource map files

[Mozzarilla]: https://github.com/MosesofEgypt/mozzarilla
[Zstandard]: https://github.com/Facebook/zstd
