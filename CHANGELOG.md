# Invader Changelog
This is used for recording Invader's changes. This changelog is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [Unreleased]
### Changed
- invader-bitmap: Spacing now attempts to sort both vertical and horizontal to see if sprites will fit in a sprite sheet

### Fixed
- invader-bitmap: Fixed some issues with spacing

## [0.9.0]
### Changed
- invader-bitmap: Changed detection to first line is blue instead of first pixel for determining when to read a whole image as one bitmap
- invader-bitmap: Removed -O (this is now done by default)
- invader-bitmap: Removed the ability to create non 1:1 sprite sheets due to them not working well with particles

## [0.8.2] - 2019-09-11
### Changed
- A changelog is now used to track changes

### Fixed
- Fixed zero spacing causing hangs

## [0.8.1] - 2019-09-11
### Added
- invader-bitmap: 3D textures are now supported

### Changed
- invader-bitmap: The cyan pixel is now optional in color plates
- invader-bitmap: Alpha is now ignored in color plates
- invader-bitmap: Textures that don't utilize blue backgrounds are now detected differently
- invader-bitmap: Sprites no longer require valid color plates
- invader-bitmap: Sprite spacing now affects the maximum number of mipmaps you can have with sprites

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
- invader-bitmap: Registration point calculation has been changed to better match tool.exe's calculations
- invader-bitmap: Alpha is now ignored when checking if a pixel is blue, magenta, or cyan

### Fixed
- invader-build: Fixed an issue with certain sounds not being played correctly, such as the "Come on! We've got to get the hell out of here!" dialogue at the start of the game

### Removed
- invader-bitmap: Removed being able to specify negative mipmaps to remove mipmaps

## [0.7.3] - 2019-09-05
### Fixed
- invader-string: Fixed cutting the last character of every string

## [0.7.2] - 2019-09-05
### Fixed
- invader-string: Fixed doubling the string every line

## [0.7.1] - 2019-09-05
### Fixed
- invader-build: unicode_string_list tags' string data is now compared when considering whether to index a tag or not

## [0.7.0] - 2019-09-04
### Added
- invader-string: Generates unicode_string_list and string_list tags

### Fixed
- invader-bitmap: Fixed pixels offset not being set multi-bitmap bitmap tags
- invader-build: Fixed CRC32 being displayed even when quiet

## [0.6.0] - 2019-08-26
### Changed
- invader-build: Some hidden values are now calculated for machines
- invader-build: All multiplayer maps use stock limits for indexed resource, while all singleplayer and UI maps can use the extended Invader resource limits

## [0.5.0] - 2019-08-24
### Changed
- invader-build: CRC32 can now be specified when building
- invader-build: CRC32 is now calculated in the header when building

### Removed
- invader-crc: CRC32 can no longer be forged using this tool; use invader-build instead

## [0.4.3] - 2019-08-23
### Changed
- invader-build: Encounter firing positions and squad positions are now raycasted for determining the BSP for an encounter
- invader-font: The width of the 'X' character is now used for the dot width.

## [0.4.2] - 2019-08-23
### Fixed
- invader-font: Fixed the dot being missing when a font is used as a console font
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
- invader-bitmap - Creates bitmap tags (only supports 2D textures without color plate data but also has custom mipmap support)
- invader-build - Builds cache files
- invader-crc - Displays and spoofs CRC32 checksums of cache files; this is useful for using modified multiplayer maps
- invader-dependency - Outputs a list of tags that depend on a given tag
- invader-indexer - Outputs the list of tags in a cache file to a text file to be used with Invader
- invader-resource - Builds resource map files
