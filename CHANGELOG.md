# Invader changelog
This is used for recording Invader's changes. This changelog is based on
[Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [Untagged]
### Added
- invader: Added support for the MCC: CEA map format
- invader-archive: Added `--verbose` which will print whether or not a tag was
  omitted as well as doing verbose comparisons.
- invader-bitmap: Added `auto` to `--format` which will default to the smallest,
  lossless output (i.e. monochrome if input is monochrome, 16-bit if it fits in
  a 16-bit color space, 32-bit otherwise)
- invader-bitmap: Added `--reg-point-hack` which sets the
  `filthy sprite bug fix` flag
- invader-bitmap: Added `--alpha-bias` which sets alpha bias
- invader-build: Added `mcc-cea` as a build target
- invader-build: Added `--resource-path` which can specify a different path to
  load resource maps from
- invader-build: Added `--resource-usage` which can specify the behavior for
  using resource maps. By default, resource maps are not used.
- invader-build: Added `--anniversary-mode` which specifies if anniversary
  assets should be used
- invader-edit: Added `--no-safeguards` which allows writing read-only data
- invader-edit: Added `--verify-checksum` which prints "matched" if the checksum
  in the header is correct or "mismatched" if not
- invader-edit: Added `--checksum` which calculates the checksum of the tag and
  prints it
- invader-edit-qt: Added viewing color plates in the bitmap previewer
- invader-info: Added `tag_order_match` which checks if a map has the same tag
  order as stock and, if not, whether it may (probably) be network compatible

### Changed
- invader: Definitions were updated to support MCC CEA season 8
- invader-archive: Tags that are excluded are now printed
- invader-bitmap: Changed the default format to `auto`
- invader-bitmap: If usage is set to alpha blend, bitmaps are now cropped if an
  edge has zero alpha and a warning will be displayed. If the resulting bitmap
  is non-power-of-two, then it will fail to generate a bitmap tag (unless the
  bitmap type is interface or sprites)
- invader-bitmap: Changed A8 to alpha-on-white instead of alpha-on-black to
  match the rendering output of the game
- invader-bitmap: Improved sprite generation to better match tool.exe (while
  still allowing sprites that exceed the limit with padding enabled but meet it
  with padding disabled)
- invader-build: `--auto-forge-target` no longer takes a parameter (it forges
  based on the input of `--game-engine`)
- invader-build: Scenarios with no scripts or globals now have their syntax and
  string data initialized
- invader-build: Changed `--build-version` to `--build-string`
- invader-edit-qt: Clicking "Find" and "Save As" for a tag now expands all
  directories to the tag's current directory
- invader-model: "Legacy" mode is now the only option as, while it's not very
  sane, the workflow of making a map currently fully depends on it. -L was also
  removed.
- invader-recover: Changed model recovery to follow the (formerly) legacy
  directory structure
- invader-recover: global_scripts are now extracted to the data folder root
- invader-sound: The default vorbis quality is now 0.8 and the default encoding
  is now 16-bit PCM.

### Fixed
- invader: Fixed a few fields in the actor tag not being shown in radians
- invader: Fixed an issue with symbolic links being followed through, resulting
  in errors
- invader: Fixed duplicate tags across multiple tags directories being
  considered in some programs
- invader: Fixed invalid enums not being properly detected if they're valid for
  HEK but marked as invalid for the game
- invader: Fixed iterating through directories with trailing slashes
- invader-archive: Fixed ignoring floating point numbers for mismatched values
- invader-archive: Fixed creating empty archives if all tags were excluded
- invader-bitmap: Fixed first bitmap index not being a valid bitmap
- invader-bitmap: Fixed an infinite loop with `-P`
- invader-compare: Fixed precision checking ignoring floating point numbers
- invader-edit-qt: Fixed "forced shader permutation" and "hud text message"
  being defaulted to -1. It is no longer allowed to set these to -1, either, as
  they are known to cause unexpected crashes.
- invader-edit-qt: Fixed a crashing issue for fast loading mode if a directory
  was present on one tags directory but not the other (if multiple tags
  directories are used)
- invader-edit-qt: Fixed switching between monochrome and RGB bitmaps in the
  bitmap previewer breaking the colors menu
- invader-model: Fixed "unnamed" not being renamed to "__unnamed". This probably
  did not do anything useful, but tool.exe does this, so we have to as well.
- invader-recover: Fixed alpha channel not being tagged correctly as an extra
  sample when making TIFFs

### Removed
- invader: Removed support for "custom_" prefixed resource maps
- invader: Removed all invader_ tag classes
- invader: Removed invader-compress
- invader: Removed support for compressed Gearbox maps
- invader-build: Removed `-a` (always index) and `-n` (no external assets) in
  favor of `-r` (use resource maps)
- invader-build: Removed `-u` (uncompressed) and `-c` (compressed)
- invader-build: Removed `-T` (tag space)
- invader-strip: Removed `-p` (preprocessing) since invader-compare has been
  able to do this anyway

## [0.44.0] - 2021-04-07
### Added
- invader-build: Added `-A` which specifies whether to forge/index against a
  specific target versus automatically doing it.
- invader-build: Added `-T` which can specify tag space
- invader-compare: Added `-j` which can let you specify the number of threads to
  do comparison. Note that doing this is a balancing act: more threads can help,
  but if I/O becomes the bottleneck (as is the case with many small tags), then
  more threads can also degrade performance. Smaller tagsets will not benefit
  from extra threads as much as larger tagsets, thus this setting is defaulted
  to 1 thread for the most consistent performance.
- invader-recover: Added `-O` to overwrite data rather than overwriting by
  default
- invader-refactor: Added `-R` which can replace strings in paths. For example,
  using `invader-refactor -R warthog puma -M move` replaces all instances of
  `warthog` with `puma`

### Changed
- invader: Re-enabled unused6, unused7, unused8, unused9 teams
- invader-build: Automatic forging and indexing now has to be manually invoked
  with `-A`
- invader-build: Unit HUD interface sequences are now checked
- invader-build: Using the high resolution scaling flag on Xbox is now treated
  as an error, as this functionality does not exist on the Xbox version
- invader-build: Some fatal errors have been reduced to errors since they are
  technically valid tag data and can be handled, but not valid for the target
  engine
- invader-build: Changed the valid parameters for `-g` to make it more
  consistent. All PC build targets are prefixed with `pc-`, and all Xbox build
  targets use the build version (i.e. `xbox-2276` for English, `xbox-0009` for
  Japanese, and `xbox-0135` for Taiwanese)
- invader-edit-qt: The tags directories are now listed in the title bar of the
  main window
- invader-edit-qt: Model node counts for the various LoDs are now hidden since
  these values are always overridden on build

### Fixed
- invader-build: Fixed an issue with -O resulting in Halo crashing
- invader-compare: Fixed a performance issue when comparing large tagsets
- invader-model: Fixed node list checksum not being copied
- invader-model: Fixed "base" permutation not being renamed to "__base"
- invader-model: Fixed marker, region, and node names not being lowercased
- invader-model: Fixed empty regions not being removed
- invader-model: Fixed some markers and triangles not being set to the correct
  region (or sometimes any region at all)
- invader-model: Fixed some triangles with -1 (null) as the shader not being set
  to shader #0, resulting in an error
- invader-model: Fixed regions starting with a tilde (~) not having the flag
  "cannot be chosen randomly" set
- invader-recover: Fixed node list checksum not being copied

### Removed
- invader-build: Dropped MCC build targets

## [0.43.0] - 2021-03-16
### Added
- New tool: invader-model - Compiles model tags
- New tool: invader-recover - Recover source data from bitmaps (if color plate
  data is present), models, string lists, tag collections, and scenario scripts
- invader-build: Added --stock-resource-bounds which, when building Custom
  Edition maps, only indexes a tag if the tag's index in the resource mapis the
  same on the equivalent stock resource map
- invader-edit: Added --list-values
- invader-resource: Added --concatenate which can concatenate an existing
  resource map's data

### Changed
- invader: Now uses long distance matching for Zstandard. This slightly improves
  compression ratio.
- invader-build: Changed --build-version's shorthand letter to `-B`
- invader-edit: Changed --list to instead show the entire structure in a TREE /F
  style. Note that this requires the terminal to support UTF-8.
- invader-edit: Changed --list to use -l (lowercase L) instead of -L which is
  used by --list-values
- invader-edit: Angles are now automatically converted to and from degrees.

### Fixed
- invader: Fixed alphabetical order for shader_transparent_chicago(_extended)
  tags
- invader-build: Fixed not defaulting pathfinding spheres for vehicles
- invader-build: Fixed `-w` not erroring if the index file couldn't be opened
- invader-edit-qt: Fixed tag subwindows (e.g. Preview bitmap") not being brought
  to front if they were already open
- invader-resource: Fixed defaulting to bitmaps and made specifying the type
  actually required.

### Removed
- invader: Removed the spheroid definitions as well as support for extracting
  and building maps with spheroid tags.
  
### Removed
- invader-resource: Removed --padding. Use --concatenate instead.

## [0.42.2] - 2021-02-10
### Fixed
- invader-edit: Fixed --list not working with arrays

## [0.42.1] - 2021-02-10
### Added
- invader-edit: Added --save-as (-O) which uses a tag path, where --output (-o)
  now uses a file path

### Fixed
- invader: Readded --fs-path. Oops.

## [0.42.0] - 2021-02-10
### Added
- New tool: invader-edit - Command-line tool for editing tags (primarily made
  for scripting)
- invader-compare: Added shorthand for the conversion types. g2m can be used in
  place of gbxmodel-to-model, m2g in place of model-to-gbxmodel, and x2c in
  place of chicago-extended-to-chicago

### Changed
- invader-bitmap: Optimized sprite placement if no budgeting is used
- invader-bitmap: Sprite sheets with multiple sprites in it now add margins to
  the sprites. We recognize that this is completely and utterly stupid due to
  how unreliable it is, but not having it breaks stock tags.
- invader-build: Bitmaps are now checked if they are supported by the target
  engine
- invader-collection: Changed input format from .tag_indices to .txt
- invader-edit-qt: Sprite count is now shown when viewing sprite sequences
- invader-edit-qt: Sprite size is now shown when viewing sprites
- invader-extract: Now uses the map type in the scenario tag for determining if
  a map is singleplayer, multiplayer, etc.

### Fixed
- invader: Fixed an exception error on some Windows shells
- invader: Fixed an issue with `-P` not working with multiple tags directories
  for tools with this option
- invader: Fixed compressed demo maps not working
- invader-bitmap: Fixed size being inaccurate in output
- invader-bitmap: Fixed sprites with double multiply usage having black borders
- invader-bitmap: Fixed regeneration error implying a bitmap tag exists if one
  does not
- invader-build: Fixed file size not being saved in the header
- invader-edit-qt: Fixed "Copy virtual path" not appearing for directories
- invader-info: Extended dirty check to check file size and type mismatching

## [0.41.4] - 2021-01-24
### Fixed
- invader-bitmap: Fixed single sprites sometimes resulting in too large of page
  sizes
- invader-bitmap: Fixed bitmap tags with multiple bitmaps breaking
- invader-sound: Fixed popping in split Xbox ADPCM sounds

## [0.41.3] - 2021-01-24
### Fixed
- invader-bitmap: Fixed sprite generation failing if no mipmap count is set
- invader-bitmap: Fixed sprite generation resulting in large sprites

## [0.41.2] - 2021-01-24
### Fixed
- invader-bitmap: Fixed an issue where some sprites would have a black border
- invader-bitmap: Fixed compressed flag not being set for DXT5 bitmaps
- invader-bitmap: Fixed encoding A8 - bitmap should be all black, not all white
- invader-build: Fixed defaulting map u/v scaling for shader_transparent_generic
  and shader_transparent_chicago maps
- invader-edit-qt: Fixed tag_collection tags not being editable if fast listing
  mode is on
- invader-edit-qt: Disabled Insert New if there are no items in an array (fixes
  a crash)
- invader-edit-qt: Fixed A8 bitmaps being displayed as an alpha blend of white
  instead of black

## [0.41.1] - 2021-01-20
### Fixed
- invader-convert: Fixed missing parameters in `-h`

## [0.41.0] - 2021-01-16
### Added
- New tool: invader-convert - Converts between various tag types (gbxmodel to
  model, model to gbxmodel, shader_transparent_chicago_extended to
  shader_transparent_chicago)
- invader-archive: Added `-e` and `-E` which check if a tag exists in a
  a specified directory (multiple directories can be specified). Using `-E`
  instead of `-e` does a functional comparison, too.
- invader-archive: Added `-O` to overwrite existing tags if `--copy` is used
- invader-build: Added `-g mcc-retail` which targets retail MCC (intended for
  CEA).
- invader-build: Added `-g xbox-tw` and `-g xbox-jp` which targets the Taiwanese
  and Japanese versions, respectively. These have a slightly higher tag space
  than the original English version and use a different build string by default.
  They also use fewer hardcoded strings.
- invader-edit-qt: Added `-L` which lets you specify `recursive` or `fast` list
  modes. Recursive listing is the old behavior which recursively scans all tags
  directories for tags, while fast listing results in the tag tree only listing
  tags when a directory is opened. The default setting is `recursive` unless you
  are on win32, which then it is `fast`. Also, filtering is only enabled while
  recursive listing is enabled.
- invader-edit-qt: Added alpha support to DXT1 decoding

### Changed
- invader: Switched DXT compression/decompression to libsquish. DXTn quality
  should be a bit better now!
- invader-build: `-E` now bumps the maximum file size of Xbox maps to 4 GiB.
  Take care that if your version of the game does not support your map's file
  size, it will crash if you try to load it.
- invader-build: Invalid text indices for item pickups and unit names/seats are
  now checked and reported as an error if invalid.
- invader-build: Using `-g mcc-retail` or `-g mcc-custom` targets 31 MiB of tag
  space
- invader-edit-qt: Optimized filtering a bit

### Fixed
- invader-archived: Fixed a segmentation fault when passing either `-g retail`
  or `-g demo`

## [0.40.1] - 2020-12-31
### Added
- invader-build: Added support for `uses demo ui`

### Changed
- invader-bitmap: Uses a new compression library for DXT compression, which adds
  color key transparency for DXT1 bitmaps
- invader-extract: Now generates mipmaps for dropped mipmaps from Xbox-extracted
  DXTn bitmap tags

### Fixed
- invader-bitmap: Fixed sub-4x4 mipmaps not being generated
- invader-build: Fixed sawtooth count (affects the strobing effect of the doors
in the levels "Truth and Reconciliation" and "Keyes")

### Removed
- invader-bitmap: Removed dithering for DXTn bitmaps (the compression library
  doesn't support it sadly!)

## [0.40.0] - 2020-12-31
### Added
- invader-archive: Added `-g` to specify game target (now required if not
  archiving single tags)
- invader-build: Added `-g xbox` engine target
- invader-build: Added `-E` to use the maximum theoretical size for the given
  cache file (4 GiB for PC, 278 MiB for Xbox due to cache partition sizes).
  Doing so may result in the cache file requiring a mod to be loaded.
- invader-build: Added `-b` to set the build version (used for Xbox maps).
- invader-bludgeon: Added `-j` for specifying thread count when using `--all`.
  On an AMD Ryzen 5 2600 with a tags directory of over 10000 tags, this reduced
  the bludgeon time from 29 seconds to 4 seconds, making it over 7x faster.
- invader-bludgeon: Added `-T invalid-uppercase-references` which detects and
  lowercases all references that contain uppercase characters
- invader-bludgeon: Added `-T excessive-script-nodes` which detects and removes
  script node tables that exceed 19001 (tags that do this potentially crash
  stock Halo PC)
- invader-extract: Added support for 3D and cube map Xbox bitmap tag extraction
- invader-index: Added feedback for if the command was successful
- invader-resource: Added feedback for if the command was successful

### Changed
- invader: Fixed segfault when querying dependencies for various tools
- invader-archive: Error checking is now (mostly) disabled when archiving a map
- invader-bitmap: Dithering can now be used on regular bitmap tags, and so can
  sprite budgets above 512x512, but you will be warned that they will not save.
- invader-build: Font tags with alternative styles references will no longer be
  indexed
- invader-build: CRC32s must now start with `0x`
- invader-build: All errors with tag data are reported as fatal errors now
- invader-build: All stock HEK limits are checked when building for all PC
  versions of Halo
- invader-build: All original file size limits are now checked when building for
  all PC versions of Halo (except MCC). Use `-E` to use the old behavior's
  higher limits.
- invader-compress: Changed compression level minimum to 0. On Deflate, this
  effectively stores the original data uncompressed and simply adds a zlib
  header, so it will always be larger than the original size, but it's nearly
  instant.
- invader-info: CRC32s are now output starting with `0x`
- invader-sound: Now uses CPU thread count by default instead of 1

### Fixed
- invader: Removed the upper bound from heat loss per second in weapon triggers;
  this will allow weapons that take less than a second to cool down to build
- invader: Fixed error reporting not checking the result on ioctl on Linux-based
  operating systems (this resulted in the line length being undefined if it
  failed)
- invader: Dropped support for uncompressed Xbox maps
- invader-bitmap: Fixed the sharpening filter being twice as powerful
- invader-bitmap: Fixed blurring being slightly more powerful than it should be
- invader-archive: Fixed errors regarding maps not being able to be read when
  not using single tag archival
- invader-build: Fixed `-n` not working as expected
- invader-build: Fixed word wrapping using the wrong length
- invader-build: Fixed manually specified BSPs for command lists and encounters
  being warned as BSP #65535 if they are completely outside of the BSP
- invader-build: Fixed warning on command lists that have zero points being
  outside of the BSP
- invader-edit-qt: Fixed subwindows always being above of the main window
- invader-info: Fixed `-h` not listing every option correctly

### Removed
- invader-bitmap: Removed `-x`
- invader-sound: Removed `-x`

## [0.39.0] - 2020-12-07
### Added
- invader-build: Added `-g mcc-custom` which builds a Custom Edition map but
  compressed and indexed for MCC while generating a .fmeta file alongside it
- invader-refactor: Added --unsafe (-U) which can be used to do a no-move
  refactor where the destination tags do not exist. This does not affect class
  refactoring.

### Changed
- invader-build: Script node count is now limited to 19001
- invader-compare: Reverted the fix from 0.38.1 because it actually broke things
- invader-edit-qt: Surface indices are now marked as `cache_only` (hidden) for
  move positions since these are calculated at build time
- invader-refactor: The destination tags must exist (unless you use --unsafe)
  when doing a no-move refactor.

### Fixed
- invader: Fixed treating Latin-1 characters as protected
- invader-build: Fixed allowing null scenario palettes (this actually crashes
  Halo!)
- invader-build: Fixed an ellipsis when too many squad positions are shown that
  aren't in the BSP
- invader-build: Fixed an issue where the wrong death animation would play
- invader-build: Fixed move positions' cluster and surface indices not being
  calculated for squads
- invader-build: Fixed some hidden values not being calculated for machines
- invader-build: Fixed the pelvis node index not being calculated for bipeds;
  this only affected biped tags which `bip01 pelvis` was not the first node, so
  stock bipeds were largely unaffected
- invader-build: Fixed a hidden value not being calculated for actor tags
- invader-build: Fixed a hidden value not being calculated for
  detail_object_collection tags
- invader-edit-qt: Leading width is now automatically calculated when previewing
  font tags
- invader-sound: Fixed "Processing sounds" hang on some systems

### Removed
- invader-build: Removed `-d` as it isn't necessary, and it produces invalid
  files anyway

## [0.38.1] - 2020-10-27
### Changed
- invader-build: Now deswizzles Xbox bitmaps on build
- invader-build: The "leading width" value in font tags can no longer be set
  manually; it is set to (ascending height + descending_height) / 5 to be in
  line with what tool.exe does
- invader-build: More defaults are now set
- invader-build: Now detects if falloff distance is greater than or equal to
  cutoff distance and both are non-zero (if so, they get set to 0 and a warning
  is shown)
- invader-edit-qt: The relative file path is now shown in the title bar rather
  than the virtual tag path
- invader-sound: No longer sets values that are defaulted when building into a
  cache file

### Fixed
- invader-build: Fixed setting "zero gain modifier" to 1 on sound classes that
  default it to 0
- invader-compare: Functional comparisons now match empty tag references

### Removed
- invader: Removed "use xbox multipurpose channel order" from definitions
- invader-edit-qt: Removed editing "leading width" for font tags since this
  value gets overwritten anyway
- invader-extract: Removed deswizzling Xbox bitmaps on extraction

## [0.38.0] - 2020-10-19
### Added
- invader-archive: Added `-C` which copies the tags to a directory instead of
  creating an archive.
- invader-bitmap: Added `-R` which uses the compressed color plate data of the
  tag in place of an image file (this will not work with extracted tags)
- invader-sound: Added `--threads` or `-j` which can be used to parallelize
  encoding and resampling of sounds with a set number of threads. This can
  drastically speed up sound tag generation when creating a sound tag with
  multiple or split permutations.

### Changed
- invader-bitmap: Height maps have been capped to 0.5 height
- invader-bitmap: Generating a height map with 0 or less height no longer
  generates a height map, and an error will be printed to the console
- invader-bitmap: If a color plate has no bitmaps in it, then it is now an error
- invader-build: Having model, gbxmodel, or scenario_structure_bsp shaders be
  set to null now fails to build, as this crashes the game
- invader-edit-qt: Mousing over a tag now displays the file size and path of the
  tag
  
### Fixed
- invader-archive: Fixed .model references being converted to .gbxmodel when
  checking for dependencies
- invader-bitmap: Fixed spacing color only working if sequence separator is
  present
- invader-bitmap: Fixed generating too few mipmaps for non-1:1 bitmaps. This was
  unlikely to cause issues, since the mipmaps missing were all one-dimensional
- invader-bitmap: Fixed not setting "disable height map compression" by default
- invader-bitmap: Fixed `-P` not working with `-R`
- invader-bitmap: Fixed an issue with loading some bitmaps
- invader-build: Fixed some scenery spawns warning about fullbright when it is
  not an issue
- invader-build: Fixed checking for local nodes when a part did not, in fact,
  have them
- invader-build: Fixed warnings regarding weapon triggers showing the wrong
  trigger indices
- invader-dependency: Fixed .model references being converted to .gbxmodel when
  checking for dependencies

## [0.37.1] - 2020-09-15
### Added
- invader-bitmap: Added `light-map` as a usage
- invader-compare: Added `-G` which allows you to ignore resource maps for the
  current input
- invader-extract: Added `-G` which allows you to ignore resource maps

### Changed
- invader-bitmap: Lightmap bitmaps no longer generate mipmaps, as expected
- invader-info: Now reports if a map's tag indices are outside of stock Custom
  Edition's
- invader-info: Changed the outputs of some of the options given

### Fixed
- invader-bitmap: Fixed mipmaps not generating

## [0.37.0] - 2020-09-10
### Added
- invader-bludgeon: Added `-T invalid-strings`
- invader-compare: Added `--by-path` or `-B` which sets how paths are compared
  This can be useful for finding duplicate tags regardless of paths. You can use
  `-B any` to match any tags regardless of tag path (useful if inputs are 
  different) or `-B different` to only match against tags with differing paths
  (useful if both inputs are the same)
- invader-edit-qt: Added a Goto menu to go to top level structs quickly
- invader-edit-qt: Added a color picker
- invader-edit-qt: Added an option to copy a virtual path without an extension
- invader-edit-qt: Added an option to copy a virtual path for a directory
- invader-refactor: Added `-M copy` which copies files without changing any
  references to the tags being copied *except* any references within the tags
  being copied

### Changed
- invader-bludgeon: Adjusted the threshold for nonnormal-vectors to trip. This
  will result in more problematic vectors being shown, but it may result in some
  vectors that are "normal enough" for tool.exe but not actually normal to show
  up, too.
- invader-build: Invalid vectors are now checked against and are considered
  errors if they are not normalized
- invader-build: String list strings that are empty or not null terminated are
  now errored (also applies to unicode string lists). This typically happens in
  tags that have been modified rather than generated.
- invader-build: Unicode string list strings that have an odd number of bytes
  are now errored
- invader-build: String lists that contain non-CRLF line endings (i.e. single \n
  or \r line endings) now result in an error. This typically only occurs in
  string lists that have been modified rather than generated.
- invader-edit-qt: Typing in a valid path with an extension into a dependency
  field will now change the class; this also applies to pasting in paths
- invader-edit-qt: A number of unused or invalid flags and enumerators have been
  hidden
- invader-edit-qt: Alphas for colors are no longer previewed by default for
  anything
- invader-refactor: Moved `--move` and `--no-move` to `-M` which is now `--mode`
- invader-string: If the `###END-STRING###` line is missing, Invader will error.
- invader-string: The format is now automatically determined by checking if the
  tag exists.

### Fixed
- invader-build: Fixed not taking rotation into consideration when checking if
  an object is in the BSP or not
- invader-edit-qt: Fixed an issue with editing ColorARGBInt values
- invader-edit-qt: Fixed an issue with names missing in arrays
- invader-info: Fixed opening protected maps resulting in an exception error

### Removed
- invader-edit-qt: Removed "mcc actor type"

## [0.36.1] - 2020-07-22
### Changed
- invader: Renamed extended_bitmap to invader_bitmap and extended_sound to
  invader_sound. The bitmap tags can simply be renamed. The sound tags can be
  renamed if they were made in 0.36.0 or newer; otherwise they have to be remade
  (as usual).

### Fixed
- invader: Fixed superclasses (e.g. object) being directly supported as
  references, since these tags do not actually exist as real tags
- invader-edit-qt: Fixed an issue where clicking "Cancel" when prompted whether
  you want to save in the tag root would still result in saving

## [0.36.0] - 2020-07-22
### Added
- invader-build: Added warnings for if you use multiple variants for a slot in a
  conversation
- invader-edit-qt: Added filtering in the Find dialog
- invader-edit-qt: Made the filter in the Find dialog and the path in the Save
  dialog have focus
- invader-edit-qt: Added selecting individual pitch ranges
- invader-sound: Added `-b` for constant bitrate for Ogg Vorbis (extended_sound
  tags will have to be remade - sorry!)

### Changed
- invader: Decompressing vertices now clears the "zoner" and "uses local nodes"
  flags since compressed vertices do not use these
- invader-bitmap: Improved the accuracy of checking bitmap tags against the
  tags in the bitmaps.map file when compiling Halo Custom Edition maps. Sprites,
  sequences, resolution, data format, type, and mipmap count are all checked.
- invader-sound: Improved the accuracy of checking sound tags against the sound
  tags in the sounds.map file when compiling Halo Custom Edition maps. Sample
  rate, channel count, format, pitch range pitches, pitch range actual
  permutation count, permutation indices, permutation gains, and skip fractions
  are now all checked.

### Fixed
- invader: Fixed decompressing vertices not calculating weight correctly
- invader: Fixed compressing vertices using local node indices instead of actual
  indices (this affects Xbox map compilation support, mainly, which isn't yet 
  implemented)
- invader-build: Fixed an issue where participants were set as valid when they
  shouldn't have (fixes unused dialogue being played in maps like b30 and a50)
- invader-edit-qt: Fixed a crash issue with Point2DInt
- invader-extract: Fixed sounds from sounds.map being extracted with the wrong
  sample rate

## [0.35.1] - 2020-07-18
### Added
- invader: Re-added checking and erroring on invalid command list indices
- invader-refactor: Added class refactoring
- invader-resource: Added `-M` which allows you to specify cache files to create
  resources

### Changed
- invader: Added support for the missing tags: continuous_damage_effect,
  multiplayer_scenario_description, preferences_network_game, and spheroid
- invader-build: Implemented "blend shared normals"
- invader-edit-qt: Monochrome bitmaps now show "Alpha-Luminance", "Luminance
  only" and "Alpha only" instead of individual color channels, since those would
  be meaningless

### Fixed
- invader-bitmap: Fixed an issue with the intensity of pixels being off-by-1 for
  truly monochromatic inputs
- invader-build: Fixed the maximum distance value not being set correctly for
  sound_looping tags that have detail sounds
- invader-build: Fixed `-u` and `-c` being ignored if specified before `-g`
- invader-compare: Fixed `-f` requiring a tag input
- invader-compare: Fixed mismatched tags showing up twice
- invader-edit-qt: Fixed "damage upper bound" showing as two separate fields
- invader-edit-qt: Fixed 2D and 3D Euler angles not showing up as degrees
- invader-refactor: Fixed `-M` not working

## [0.35.0] - 2020-07-14
### Added
- invader-bludgeon: Added `-T nonnormal-vectors` which can detect and fix
  potential vectors that tool.exe does not like when building lightmaps
- invader-build: Added word wrapping for most error messages when on Linux or
  Windows
- invader-build: Added some minor warnings for weapons
- invader-compare: Added `-f` which will check if tags are functionally
  identical by compiling the tag beforehand (this will be slower)
- invader-edit-qt: Added an error message for if a subdirectory could not be
  queried
- invader-edit-qt: Added a filter option in the View menu to filter out tags
  that don't match a given expression
- invader-edit-qt: Added support for model tags (as opposed to gbxmodel)
- invader-edit-qt: Added support for hexadecimal input (integers only)
- invader-edit-qt: Added comments for some fields in weapons.
- invader-edit-qt: Added an icon courtesy of ST34MF0X
- invader-extract: Xbox model tags now extract
- invader-extract: Added word wrapping for most error messages when on Linux or
  Windows
- invader-font: Added `-l` to limit to the first 256 characters
- invader-resource: Added `-p` to specify a number of extra bytes to add after
  the header when generating resource maps
- invader-resource: Added `-w` to specify resource indices rather than the
  default Custom Edition resource indices

### Changed
- invader: Help menus will adapt to the user's terminal width if the user is on
  either Linux or Windows and their terminal width is at least 80 characters
- invader: "dark" and "Dark Circlet" have been renamed to "native" and
  "Invader (native)", respectively
- invader: Invalid indices in command lists are no longer considered an error,
  as "fixing" them can result in unintended consequences on some campaign maps
- invader: Maps will no longer load if they contain invalid paths
- invader-build: Now warns if scenery and light fixtures were found outside of
  the BSP, as they won't spawn if they were
- invader-build: Maps that error due to multiple objects sharing the same name
  now list the objects with the name to help you track them down
- invader-build: Non-power-of-two bitmaps now result in a pedantic warning
  instead of an error
- invader-build: Now looks for custom_bitmaps.map, custom_sounds.map, and
  custom_loc.map and uses those if present when building Custom Edition maps
- invader-build: The final CRC32 now factors in bitmap/sound data (unless you
  decide to forge the CRC32)
- invader-build: The commit hash is no longer included in cache files due to low
  space
- invader-build: If building a Halo Custom Edition map and tags are detected to
  not match a tag found in a resource map with the same path, then a minor
  warning will be emitted (does not apply if using `-a` or `-n`)
- invader-build: Now warns if shader_transparent_generic tags are used upon
  building for the Gearbox port
- invader-build: Elements in weapon HUD interfaces now set the zoom bit even if
  no crosshair in the tag is a zoom crosshair and it uses one of the three zoom
  flags (the respective warning is removed)
- invader-compare: Using `-s matched` or `-s mismatched` no longer prints the
  `Matched: ` or `Mismatched: ` prefixes, nor does it show the count matched or
  mismatched. This should make it useful for piping to a program or text file.
- invader-compress: Now shows the compression format used
- invader-edit-qt: Indices now default to 65535 on initialization
- invader-edit-qt: Null indices now have placeholder text, and null indices can
  be empty
- invader-edit-qt: Changed the name of some fields in triggers for consistency
- invader-extract: Now looks for custom_bitmaps.map, custom_sounds.map, and
  custom_loc.map and uses those if present when extracting Custom Edition maps
- invader-extract: No longer attempts to open resource map files when attempting
  to extract a map that doesn't support them
- invader-refactor: -T is now required for refactoring individual tags, but you
  can use it multiple times in a single invocation
- invader-refactor: -D must now be specified with -N or -M
- invader-resource: Replace `-R` with `-g` which is now required
- invader-resource: Custom Edition maps are now prefixed with "custom_" by
  default for the purpose of extracting both Custom Edition and retail Halo PC
  maps w/in the same maps directory; use `-n` to disable this behavior
- invader-resource: Building loc files with an index no longer requires an
  explicit extension in the index, and if one isn't present, then it will search
  for the tag to find the extension
- invader-string: Empty string list tags can no longer be created

### Fixed
- invader: Added an enum in the actor_variant tags for MCC's scoring
- invader: Fixed the ColorARGBInt definition (you do not need to re-extract tags
  or rebuild maps for this - it effectively only impacted invader-edit-qt)
- invader: Fixed a couple issues with compressed vertex generation, including an
  issue with null part indices as well as an issue with part indices exceeding
  42 (so if it exceeds 42, Invader will not attempt to generate compressed
  vertices)
- invader: Fixed Xbox compression not aligning to a 4096-byte boundary. This
  would result in Halo refusing to load the map.
- invader-bitmap: Fixed an issue with DXT3 making some fully opaque bitmaps
  semi-transparent
- invader-bludgeon: Fixed an issue where it would warn about model nodes missing
  compressed vertices if a part node exceeds 42 (it is impossible to compress
  vertices with more than 42 parts)
- invader-bludgeon: Fixed printing "no issues detected" after printing issues
  that were detected
- invader-build: Fixed an issue where filthy parts were not saved, resulting in
  the FP needler core not glowing (you will need to re-extract any GBXModel tags
  that were affected by this issue)
- invader-build: Fixed some issues with scenery not spawning on MCC
- invader-build: Fixed uncompressed size having the wrong percentage
- invader-build: Fixed an issue with rain not appearing in some maps
- invader-build: Fixed an issue with detail objects not appearing in some maps
- invader-build: Uncompressed bitmaps marked as compressed now error
- invader-build: Fixed bend bounds defaulting to 0-1 rather than being based on
  natural pitch (you will get a pedantic warning if it does this and your bend
  bounds are not 0)
- invader-build: Fixed decal radius upper bound not being defaulted correctly
- invader-build: Fixed damage effect maximum intensity and camera shaking wobble
  period not defaulting to 1
- invader-build: If a tag class is unimplemented, a more descriptive error is
  shown
- invader-build: Fixed a crash with some maps that have fog planes such as d40
- invader-edit-qt: Fixed crashing if a tag directory contained files that could
  not be accessed in the filesystem
- invader-edit-qt: Fixed 8-bit color being displayed in the wrong order
- invader-edit-qt: Fixed 2D rectangle edits crashing
- invader-edit-qt: Fixed a segmentation fault when failing to open a tag
- invader-edit-qt: Fixed "Cancel" discarding when closing a modified file
- invader-edit-qt: Fixed "acceleration time" and "deceleration time" having
  missing units
- invader-extract: Fixed an issue with some sound tags not extracting correctly
  when extracting from specifically modified Custom Edition maps
- invader-index: Fixed not including .bitmap or .sound in bitmaps.map or
  sounds.map, respectively
- invader-resource: Fixed building Custom Edition sounds.map files having
  partially cut off sounds in sounds with multiple permutations
- invader-sound: Fixed an issue with generating split sound tags not setting the
  actual permutation count correctly as a result of a past optimization

### Removed
- invader: Removed MCC support
- invader-bludgeon: Removed `-T invalid-power-of-two`
- invader-build: Removed automatically converting model references to gbxmodel
  (required to support Xbox maps and assets which don't use gbxmodel)

## [0.34.1] - 2020-04-12
### Changed
- invader-extract: Readded extraction via ipaks for MCC maps

### Fixed
- invader-extract: Fixed an issue with decompression errors if loading an MCC
  map with a bitmaps.map file present

## [0.34.0] - 2020-04-12
### Added
- invader-build: Added `-d` which discards all raw data. This will make the map
  unusable in clients (except MCC), but it can be used for saving space.

### Changed
- invader: Optimized font rendering to utilize character tables
- invader: Improved accuracy for how MCC maps are loaded
- invader-build: Building MCC maps now includes all resources in the map again.
  Use `-d` to change this behavior to what it was before. This was done for
  consistency reasons.

### Fixed
- invader-build: Changed how BSP vertices are stored for MCC maps, fixing how
  some maps with multiple BSPs are loaded

## [0.33.4] - 2020-04-11
### Added
- invader-edit-qt: Added string previewing

### Changed
- invader: Extraneous `begin` blocks are now removed on script decompilation

### Fixed
- invader-build: Object names referenced only by "set new name" are no longer
  warned for being unused

## [0.33.3] - 2020-04-07
### Fixed
- invader-bitmap: Fixed `--usage` being missing
- invader-edit-qt: Fixed hidden, cache-only options being visible in flags

## [0.33.2] - 2020-04-07
### Changed
- invader: Unknown flags are now stripped on load
- invader-edit-qt: Changed the default text for font previewing

### Fixed
- invader-edit-qt: Fixed characters not appearing if they're cut off
- invader-edit-qt: Fixed font preview not being updated if font tag is modified

## [0.33.1] - 2020-04-06
### Added
- invader-edit-qt: Added font previewing

### Fixed
- invader-compare: Fixed crashing on maps with stubbed or invalid tags

## [0.33.0] - 2020-04-05
### Added
- New tool: invader-compare: Compares tags between maps and/or tags directories

### Changed
- invader-build: More hidden values are now set

### Fixed
- invader: Fixed color codes being used on Linux if stdout/stderr is not a TTY
- invader-build: Fixed a segmentation fault when using certain corrupt
  bitmaps.map and sounds.map files when building a Custom Edition map without
  `-a`
- invader-build: Fixed `-a` rejecting all bitmaps instead of matching them all
- invader-resource: Fixed generated Custom Edition bitmaps.map being corrupted
- invader-resource: Fixed generating Custom Edition sounds.map segfaulting

### Removed
- invader-resource: Removed campaign assets from the internal list of tags to
  put in resource maps

## [0.32.5] - 2020-04-01
### Added
- invader-bludgeon: Added `-T invalid-indices` which nulls various invalid
  indices. Note that this may result in various Bungie assets working slightly
  differently, as these have several errors that tool does not check.

### Changed
- invader-build: Now errors on more invalid indices

## [0.32.4] - 2020-03-31
### Changed
- invader: Dependencies with zero-length paths now get defaulted to the default
  tag class if one is present

### Fixed
- invader-build: Fixed pathfinding sphere offsets not being set correctly

## [0.32.3] - 2020-03-30
### Fixed
- invader-build: Fixed pathfinding spheres not being generated on build for some
  tags

## [0.32.2] - 2020-03-28
### Fixed
- invader-build: Fixed an issue with lightning tags not rendering

## [0.32.1] - 2020-03-27
### Fixed
- invader-build: Fixed a segfault issue with 3D firing positions

## [0.32.0] - 2020-03-27
### Added
- invader-bludgeon: Added `-T missing-script-source` which decompiles scripts in
  scenario tags.
- invader-build: Added child scenario merging
- invader-edit-qt: Left-clicking an array name replaces the combo box with a
  spin box so you can type an index in directly

### Changed
- invader-build: Now checks extension separately from scenario name
- invader-build: Maps with script data can no longer be built without source
  data. This is due to the fact that such scenario tags are going to be broken.
- invader-build: Firing positions, squads, and command list points that fall out
  of BSPs are now listed
- invader-compress: `--level` now applies to MCC/Xbox
- invader-extract: Now always decompiles scripts
- invader-edit-qt: Invalid values are displayed as red, and missing dependencies
  are displayed as orange
- invader-index: Renamed invader-indexer to invader-index to be more in line
  with the naming scheme of Invader tools

### Fixed
- invader-build: Fixed the check for child scenarios; these still don't work
- invader-build: Fixed some command lists not working correctly due to the point
  surface indices not being calculated. This may not fix all of them, so stay
  tuned for more changes.
- invader-build: Greatly improved the accuracy of firing positions so everything
  (or nearly everything) firing position related works
- invader-build: Fixed some AI conversations not playing
- invader-edit-qt: Fixed tag count not being updated on the main window if the
  tags directory is empty
- invader-edit-qt: Fixed an unnecessary warning appearing on Windows when
  saving newly-created tags
- invader-edit-qt: Fixed a crash that occurred on creating a tag and directories
  for the tag failing to be created

## [0.31.0] - 2020-03-24
### Added
- invader-bludgeon: Added `-T incorrect-sound-buffer` which fixes incorrect
  sound buffer sizes set in the tag (can fix instances of sounds being the
  incorrect length when played in-game)
- invader-bludgeon: Added `-T missing-vertices` which fixes (un)compressed
  vertices being missing; not having these present in BSP tags can result in
  errors when performing lightmap generation
- invader-bludgeon: Added `-T invalid-reference-classes` which nulls references
  with invalid references; more functionality is planned for this later
- invader-bludgeon: Added `-T invalid-power-of-two` which marks non-power-of-two
  bitmaps as power-of-two and vice versa
- invader-bludgeon: Added `-T out-of-range` which clamps values that are outside
  of their respective ranges
- invader-build: Added a warning for using `-o` and outputting a map that does
  not match the scenario tag (unless building for MCC)

### Changed
- invader: Changed the internal Dark Circlet map format to use 64-bit offsets
- invader: Removed checking for invalid values when compiling a tag outside of
  invader-build (i.e. recursion in invader-dependency, invader-extract, etc.)
- invader-bitmap: extended_bitmap tags now store compressed color plate data in
  Zstandard
- invader-bludgeon: Now counts bludgeoned tags as tags that were changed
- invader-bludgeon: No longer shows unaffected tags if using `-a`
- invader-edit-qt: Changed default dimensions of tag windows to scale better
  with larger tags
- invader-edit-qt: Minimums/maximum values are now enforced
- invader-edit-qt: Added overflow protection for values
- invader-sound: Reduced memory consumption

### Fixed
- invader: Fixed a parsing error with child scenarios
- invader-bitmap: Fixed non-power-of-two bitmaps being marked as power-of-two
- invader-build: Fixed an error message for sound permutations
- invader-build: Fixed a deprecation warning with missing source data even when
  source data was present
- invader-build: Fixed BSP sizes not being checked against tag space when
  building retail/custom/demo maps
- invader-build: Fixed some potential segmentation faults when building maps
  with `-g dark`
- invader-edit-qt: Fixed short sounds not playing in sound previewing
- invader-sound: Fixed non-extended sounds being named extended_sound

## [0.30.1] - 2020-03-15
### Fixed
- invader-edit-qt: Fixed bitmaps with non-equal height & width being cut off
- invader-edit-qt: Fixed "Original" scaling actually being 2x

## [0.30.0] - 2020-03-15
### Added
- New tool: invader-bludgeon - Tool that convinces tags to work with Invader
- New class: extended_bitmap - Contains all extra features in invader-bitmap
  that are not present in stock tool.exe (use `-x` to enable)
- New class: extended_sound - Contains all extra features in invader-sound
  that are not present in stock tool.exe (use `-x` to enable)
- invader-edit-qt: Added two new buttons to arrays: "Insert" and "Delete All".
  "Insert" now does what "Add" did, and "Delete All" now does what "Clear" did.
- invader-edit-qt: Added `--no-safeguards` to allow editing of any field (don't
  use this feature)
- invader-edit-qt: Added opening tags if a path is given when opening the
  program
- invader-edit-qt: Added the ability to toggle fullscreen for tag editing
- invader-edit-qt: Added bitmap previewing, complete with sprite highlighting,
  color channel viewing, nearest-neighbor upscaling, and linear downscaling
- invader-edit-qt: Added sound previewing
- invader-edit-qt: Added naming to some arrays
- invader-extract: Added MCC CEA bitmap tag extraction

### Changed
- invader-bitmap: Added support for 16K sprite plates
- invader-bitmap: Added support for 1K and 2K budgets for sprites
- invader-bitmap: Moved >512x512 sprite budgets, dithering, and nearest-neighbor
  to extended_bitmap
- invader-edit-qt: Improved styling and UX
- invader-edit-qt: Bitfields can no longer be highlighted
- invader-edit-qt: Limits for certain tags are enforced
- invader-edit-qt: Some elements have been marked as read-only. This isn't as
  restrictive as Guerilla, but there are some elements that, if edited, will
  likely lead to the tag not working in invader-build or Halo. Use
  --no-safeguards to override.
- invader-edit-qt: The "Clear" button now clears all fields rather than deleting
  all elements from the array
- invader-edit-qt: The "Add" button now appends elements to the bottom of the
  array similar to what Guerilla does
- invader-edit-qt: Opening a file now displays in the status bar
- invader-sound: Specifying the format is now required whenever sound data is
  being put into a map
- invader-sound: Moved forcing sample rate and channel count to extended_sound,
  as well as using FLAC tags or creating sound tags without specifying a format

### Fixed
- invader: Fixed some issues with outputting text on various installations of
  Windows
- invader-build: MCC maps no longer include bitmap or sound data inside of the
  cache file since it isn't even used. This should make maps smaller.
- invader-edit-qt: Fixed a performance issue on Windows
- invader-edit-qt: Fixed out-of-bounds enums crashing. For tags that have
  invalid enums, you should use invader-bludgeon to resolve them.
- invader-edit-qt: Fixed closing modified tags crashing if "Save" was clicked
- invader-edit-qt: Fixed memory not being freed when closing a tag until
  another tag was opened
- invader-edit-qt: Fixed an issue where files >2 GiB would not open on Windows;
  this was due to filesystem::file_size() being bugged in the fs implementation

### Removed
- invader-bitmap: Removed dithering red, green, and blue channels individually.
  Instead, you use "a", "rgb", or "argb" for colors.

## [0.29.0] - 2020-03-04
### Added
- invader-edit-qt: GUI tool for editing tags
- invader-extract: Added partial support for Xbox maps
- invader-info: Added partial support for Xbox maps

### Fixed
- invader-bitmap: Fixed TIFFs without alpha being read as black
- invader-build: Fixed incorrect CEA tag space limit being enforced

## [0.28.1] - 2020-02-25
### Changed
- invader-build: Halo CEA tag space limits are now enforced (31 MiB tag space,
  not including BSPs)

### Fixed
- invader-build: Actually changed `anniversary` to `mcc`
- invader-build: Fixed sound_scenery references not working in effect tags
- invader-build: Fixed some bounds checking issues when compiling invalid
  collision models or BSP collision
- invader-build: Fixed scenery and light fixtures not spawning on MCC maps
- invader-extract: Fixed an issue where certain model_animation tags did not
  extract

## [0.28.0] - 2020-02-24
### Added
- New tool: invader-collection - Generates tag collection tags

## [0.27.1] - 2020-02-24
### Added
- invader-build: Added --uncompressed for engine targets that default to
  compressed (i.e. `mcc`) to be built as uncompressed.

### Changed
- invader-build: Changed `anniversary` to `mcc` for brevity

## [0.27.0] - 2020-02-24
### Added
- invader-info: Added `-T uncompressed-size`
- invader-build: Added `anniversary` as an engine target. If building MCC CEA
  maps, you will need to modify the .fmeta tag, too

### Fixed
- invader-build: Fixed maps with stubbed tags being detected as protected in
  invader-info; note, however, that maps will need to be rebuilt

## [0.26.1] - 2020-02-21
### Added
- invader-extract: Can now extract BSP tags from CEA maps.

## [0.26.0] - 2020-02-21
### Added
- invader-compress: Can now decompress/compress CEA maps. If decompressed by a
  tool besides invader-compress, then the engine version in the header will
  need to be changed from 0x7 (7) to 0x233 (563) to correctly recompress. The
  value will be set back to the correct value upon compression.
- invader-extract: Can now extract tags from CEA maps. If the map is
  uncompressed from a tool besides invader-compress, then the engine version in
  the header will need to be changed from 0x7 (7) to 0x233 (563) to correctly
  load. BSP tags, sounds, and bitmaps do not currently extract.
- invader-indexer: Can now index tags from CEA maps. If the map is
  uncompressed from a tool besides invader-compress, then the engine version in
  the header will need to be changed from 0x7 (7) to 0x233 (563) to correctly
  load.
- invader-info: Can now query CEA maps. If the map is uncompressed from a tool
  besides invader-compress, then the engine version in the header will need to
  be changed from 0x7 (7) to 0x233 (563) to correctly load.

## [0.25.2] - 2020-02-12
### Added
- invader-refactor: Added `-D` which performs the operation without making any
  actual changes
- invader-refactor: Added `-s` which only writes to a single tag

### Changed
- invader-build: Now errors if any tag has an animation graph but no model
- invader-refactor: Some exceptions are now handled by gracefully closing
  rather than calling abort() and crashing.

### Fixed
- invader-archive: Fixed an issue with single tag archival when archiving
  bitmaps or sounds
- invader-extract: Fixed an issue where some hidden values were being stripped
  unnecessarily
- invader-strip: Fixed an issue where some hidden values were being stripped
  unnecessarily

## [0.25.1] - 2020-02-12
### Changed
- invader-build: Now warns if a biped or vehicle is missing an animation graph,
  and, if not, errors if the tag is missing a model

### Fixed
- invader-build: Fixed a typo in an error message for weapon_hud_interface tags
- invader-build: Fixed showing the wrong indices in weapon_hud_interface errors
- invader-build: Fixed non-power-of-two check giving incorrect results when
  compiling bitmaps
- invader-build: Fixed a missing "bip01 head" resulting in a warning

## [0.25.0] - 2020-02-11
### Added
- New tool: invader-refactor - Find and replace tag references.
- invader-strip: Added `-p` which runs the tag through Invader's preprocessor
  (used in invader-build). This is to make tags easier to compare.

### Changed
- invader: gbxmodel tags that have markers located in the main model struct are
  now considered invalid
- invader: Tags with multiple consecutive path separators are now fixed when
  run through the tag or map parser

## [0.24.2] - 2020-02-01
### Fixed
- invader-build: Fixed ejection port recovery rate being calculated as 1 for
  some tags

## [0.24.1] - 2020-01-31
### Added
- invader-build: Now shows tag space usage on map build and errors if exceeded

### Changed
- Node count is now capped to a maximum of 255
- invader-build: Adds markers in the same order as tool.exe
- invader-build: Node indices are now checked
- invader-extract: Now recalculates detail node count on extraction
- invader-extract: Extracts markers in the reverse order that tool.exe adds
  them
- invader-extract: Removes the `blend_shared_normals` flag on extraction to
  prevent generational loss (since tool.exe may change the normals again!)
- invader-strip: Default values are no longer automatically set

### Fixed
- invader-build: Fixed "zoner" gbxmodel tags not having the correct detail node
  counts
- invader-extract: Fixed change color permutation weights being extracted
  incorrectly. **You will need to re-extract your tags!!!**
- invader-build: Fixed change color permutation weights not being converted to
  cutoffs.

## [0.24.0] - 2020-01-22
### Added
- invader-strip: Added `-a` which strips the entire tags directory

### Changed
- invader-dependency: `-r` and `-R` no longer show tag errors
- invader-extract: `-r` no longer shows tag errors

## [0.23.5] - 2020-01-22
### Fixed
- invader-build: Fixed a bug where negative-scale weapon HUD interface values
  were NOT defaulted to 1

## [0.23.4] - 2020-01-22
### Fixed
- invader-build: Fixed various maps with decals crashing

## [0.23.3] - 2020-01-21
### Changed
- invader-build: Detail node counts for gbxmodels are now calculated.
- invader-sound: Permutations are now alphabetized. For split sounds, only the
  "actual" permutations are alphabetized.

### Fixed
- invader-build: Fixed some weapon HUD interface tags not compiling due to
  finding an error when there was none
- invader-build: Fixed some various division-by-zero issues
- invader-extract: Fixed some values being extracted when unnecessary
- invader-strip: Fixed some values not being stripped

## [0.23.2] - 2020-01-21
### Changed
- invader-strip: Now defaults render bounding radius
- invader-strip: Now strips out dependencies from meter tags since they're not
  used

### Fixed
- invader-extract: Fixed an issue where certain singleplayer tags' values were
  not restored to the original HEK values such as pistol.weapon's error angle
- invader-extract: Fixed not clearing do_not_cull flag
- invader-build: Fixed defaulting height/width scales for interface numbers in
  weapon HUD interfaces to 1
- invader-strip: Fixed defaulting height/width scales for interface numbers in
  weapon HUD interfaces to 1

## [0.23.1] - 2020-01-20
### Fixed
- invader-strip: Fixed an issue with not compiling on Windows due to a
  difference in the standard library implementation.

## [0.23.0] - 2020-01-19
### Added
- New tool: invader-strip - Strips extra hidden data from tags.
- invader-build: Added a CRC32 check on tag load. This will drastically
  increase build time, but it will still be much faster than tool.exe. You can
  turn this off by turning off pedantic warnings (`-H`) or reset the CRC32 with
  invader-strip.

## [0.22.4] - 2020-01-17
### Added
- invader-info: Added `-T tags-external-bitmap-indices` which lists all bitmap
  tags in a Custom Edition map that use an external index
- invader-info: Added `-T tags-external-indices` which lists all tags in a
  Custom Edition map that use an external index
- invader-info: Added `-T tags-external-loc-indices` which lists all tags in a
  Custom Edition map that aren't sounds or bitmaps but use an external index
- invader-info: Added `-T tags-external-sound-indices` which lists all sound
  tags in a Custom Edition map that use an external index

### Changed
- invader-sound: Warns if indexed damage effect or object tags are stubbed out

### Fixed
- invader-build: Fixed decals not being put in the correct cluster on some maps
- invader-build: Fixed camera shake wobble period not being converted to ticks
- invader-build: Fixed resource bitmap tags with different bitmap data count
  matching when they shouldn't when building for Halo Custom Edition
- invader-extract: Fixed camera shake wobble period not being converted from
  ticks
- invader-resource: Fixed a segmentation fault when building resource maps

## [0.22.3] - 2020-01-15
### Added
- invader-build: Minor warnings are now considered "pedantic" and can be hidden
  with `-H`. You should probably still fix these warnings, as you may be
  getting an undesirable result.
- invader-build: Added warnings for non-power-of-two bitmaps being incorrectly
  set as power-of-two and vice versa.
- invader-build: Stock multiplayer maps for demo, retail, and Custom Edition
  are now automatically indexed to prevent crashing when joining with modified
  maps.
- invader-build: Stock multiplayer maps for Custom Edition now have their CRC32
  automatically forged to whatever the stock CRC32 is to allow users to join
  other servers with modified maps.
- Added library-wide Ogg Vorbis and FLAC decoding and encoding

### Changed
- invader-sound: Disabled the creation of split dialogue. This does not work
  well with the game, anyway.
- invader-sound: Older sound tag data is now copied over.
- invader-build: Specifying the target engine is now required when building a
  map.
- invader-build: If building a Custom Edition map with a stock scenario name,
  the following values are set:
    - `vehicles\rwarthog\rwarthog_gun` autoaim angle: 6 degrees
    - `vehicles\rwarthog\rwarthog_gun` deviation angle: 12 degrees
    - `vehicles\banshee\banshee bolt` stun: 0
    - `vehicles\banshee\banshee bolt` max stun: 0
    - `vehicles\banshee\banshee bolt` stun time: 0
    - `vehicles\ghost\ghost bolt` stun: 0
    - `vehicles\ghost\ghost bolt` max stun: 0
    - `vehicles\ghost\ghost bolt` stun time: 0
- invader-build: If building a retail or `dark` map with a stock scenario name,
  the following values are set:
    - `vehicles\rwarthog\rwarthog_gun` autoaim angle: 1 degree (6 if `dark`)
    - `vehicles\rwarthog\rwarthog_gun` deviation angle: 1 degree (12 if `dark`)
    - `vehicles\banshee\banshee bolt` stun: 1
    - `vehicles\banshee\banshee bolt` max stun: 1
    - `vehicles\banshee\banshee bolt` stun time: 0.15
    - `vehicles\ghost\ghost bolt` stun: 1
    - `vehicles\ghost\ghost bolt` max stun: 1
    - `vehicles\ghost\ghost bolt` stun time: 0.15
- invader-build: If a value is changed automatically due to the map being a
  singleplayer or stock map, a warning is now displayed.

### Fixed
- invader-build: Warning for non-power-of-two was fixed (it warned only if it
  was an interface bitmap, not only if it was NOT an interface bitmap)
- invader-sound: Fixed std::terminate being called on an invalid WAV being
  used.
- invader-sound: Fixed not loading .wav files with fmt subchunks that are
  larger than standard.

## [0.22.2] - 2020-01-10
### Fixed
- invader-extract: Fixed recursive extraction causing segmentation faults with
  some tags

## [0.22.1] - 2020-01-10
### Added
- Added color support for Windows

### Changed
- invader-build: Improved the accuracy of building
- invader-bitmap: Now uses the new parser
- invader-font: Now uses the new parser
- invader-string: Now uses the new parser
- invader-string: Changed "utf-16" to "unicode"
- Compiling with `-DINVADER_EXTRACT_HIDDEN_VALUES` now disables some programs

### Fixed
- invader-build: Fixed an out-of-bounds issue resulting in undefined behavior

## [0.22.0] - 2020-01-08
### Added
- invader-build: Added `-O` which can reduce tag space usage
- invader-build: Added numerous warning and error checks which can be used to
  spot and report problems with maps
- invader-build: Added more helpful text output

### Changed
- invader-build: Improved the accuracy of encounter firing position location

### Fixed
- invader-build: Fixed various light tags not working correctly
- invader-build: Fixed sound_looping gain not being defaulted to 1

## [0.21.4] - 2020-01-04
### Changed
- invader-extract: Tag IDs are now set to FFFFFFFF when extracted. This will
  make extracted tags more reproducible independent of the map being extracted.
- invader-extract: The maps folder the map is in is used if no maps folder is
  supplied

### Fixed
- All programs: Fixed an easy-to-reproduce segmentation fault crash on Windows.

## [0.21.3] - 2019-12-26
### Fixed
- invader-build: Fixed converting radians to degrees with the lens_flare
  rotation scale
- invader-extract: Fixed converting degrees to radians with the lens_flare
  rotation scale

## [0.21.2] - 2019-12-26
### Fixed
- invader-build: Fixed an issue with some old device_light_fixture tags being
  treated as device_control tags. This generally did not result in gameplay
  issues, but it may have resulted in a prompt to "activate" it when the player
  was close to the object.

## [0.21.1] - 2019-12-25
### Added
- invader-sound: Added `--channel-count` to allow you to specify the channel
  count. If you specify mono, this will mix down all stereo to mono for you.
- invader-sound: Added support for 32-bit float PCM audio

## [0.21.0] - 2019-12-25
### Added
- New tool: invader-sound - Generates sound tags
- Added colors for various diagnostic messages depending on the user's terminal
- invader-info: Added `-T external-tags` which indicates the number of tags
  that use external resource maps, including indexed tags
- invader-info: Added `-T external-bitmaps` which indicates the number of
  bitmap tags that use external resource maps, including indexed tags
- invader-info: Added `-T external-loc` which indicates the number of loc tags
  that use external resource maps, including indexed tags
- invader-info: Added `-T external-sounds` which indicates the number of sound
  tags that use external resource maps, including indexed tags
- invader-info: Added `-T external-bitmap-indices` which indicates the number
  of externally indexed bitmap tags
- invader-info: Added `-T external-loc-indices` which indicates the number of
  externally indexed loc tags
- invader-info: Added `-T external-sound-indices` which indicates the number of
  externally indexed sound tags
- invader-info: Added `-T languages` which lists all languages valid for the
  map separated with spaces
- invader-info: Added `-T external-pointers` which indicates whether the map
  uses external pointers (sometimes occurs if built from tool.exe due to a bug
  with indexing raw data)
- invader-info: Added `-T tags-external-pointers` which list tags that have
  external pointers that were not indexed

### Changed
- invader-info: Now outputs some details in color on some terminals

## [0.20.2] - 2019-11-27
### Changed
- Enabling hidden value extraction now produces unusable tag files to
  discourage users from using them in cache files
- parser.cpp was split up into multiple files to improve compilation time when
  making using multiple jobs
- invader-extract: BSP trigger volumes are now deleted on extraction since they
  are generated on map build
- invader-extract: Dependency and output tag file paths are now made lowercase
- invader-build: Index parsing is now case insensitive

### Fixed
- invader-extract: Fixed unknown tags outputting as 0 byte files

## [0.20.1] - 2019-11-26
### Changed
- invader-bitmap: Now fails if the tags directory isn't valid
- invader-extract: Now fails if the tags directory isn't valid
- invader-font: Now fails if the tags directory isn't valid
- invader-string: Now fails if the tags directory isn't valid

### Fixed
- invader-build: Fixed setting mouth data and subtitle data to 0 size
- invader-build: Fixed incorrect footstep sounds being used if not using an
  MEK-extracted BSP tag

## [0.20.0] - 2019-11-26
### Added
- invader-extract: Added `--non-mp-globals` / `-n` which is required to extract
  globals tags from non-multiplayer maps, as there is little value in building
  cache files with these tags, and you will need to add the missing multiplayer
  information block, yourself

### Changed
- invader-build: Strips default data from compressed animations now

### Fixed
- invader-extract: References are cleared from scenario tags
- invader-extract: Fixed an error when extracting compressed animations
- invader-extract: Fixed sound tags' song length values not being preserved
- invader-extract: Fixed cutscene text fade taking 900x as long as expected
- invader-extract: Fixed multiplayer scenario descriptions not being openable
- invader-extract: Fixed some BSP trigger volumes not triggering properly
- invader-extract: Fixed the Vorbis sample count not being copied correctly
- invader-extract: Fixed some animation issues

## [0.19.2] - 2019-11-24
### Added
- invader-extract: Added recursive extraction

### Changed
- invader-extract: There is no longer any default maps folder
- invader-extract: Some tags are now reset back to what they are originally if
  extracting from a single player map:
    - `weapons\pistol\pistol.bullet`
        - Elite energy shield damage modifier set to 1.0
    - `weapons\pistol\pistol.weapon`
        - Error angle set to 0.2° - 2.0° in the first trigger
        - Minimum error set to 0.0° in the first trigger
    - `weapons\plasma rifle\plasma rifle.weapon`
        - Error angle set to 0.5° - 5.0° in the first trigger
- invader-extract: Globals tags in non-multiplayer maps are no longer extracted
  unless you use `-s`, and this is due to those tags having data stripped out
- invader-extract: Tags with with `..` and `.` directories will no longer be
  extracted as these are potentially dangerous

### Deprecated
- invader-extract: Hidden values are no longer extracted by default. A compiler
  flag was added to do this, but it will be removed at a later version.

### Removed
- invader-extract: Removed -c (--continue), making it always on
- invader-extract: Removed -n (--no-external-tags), making it effectively on if
  you don't specify a maps folder

## [0.19.1] - 2019-11-23
### Changed
- invader-extract: Extracting all tags now prints all tags that were extracted
- invader-extract: Extracting searched tags now prints the time it took

### Fixed
- invader-extract: Fixed an issue with extracting internalized sounds
- invader-extract: Fixed an issue with extracting uncompressed audio

## [0.19.0] - 2019-11-23
### Added
- New tool: invader-extract - Extracts tags from cache files
- Added a new tag parser that programs can use

### Changed
- libTIFF and CMake 3.12 are now required to compile Invader

### Fixed
- invader-build: Fixed light tag durations not being properly multiplied by 30
- invader-build: Fixed some sound tags not working as intended
- invader-build: Fixed lens flare rotation scale not being converted properly

## [0.18.0] - 2019-11-17
### Changed
- All tag definitions have been converted to .json format .This will allow them
  to be used for even more purposes than before.

### Fixed
- invader-build: Fixed `-g` not erroring if an invalid engine was given
- invader-build: Fixed sound looping tags' gain being set to 0
- invader-build: Fixed some values in shader_transparent_plasma being set to 0
- invader-build: Fixed biped A In, B In, C In, D In being set to 0
- invader-build: Fixed some objects' flags being set to 0
- invader-build: Fixed an undefined behavior issue with firing positions

## [0.17.0] - 2019-11-13
### Added
- invader-indexer: Added `--info` to invader-indexer to show information about
  Invader, similar to other Invader programs
- invader-font: Added support for .otf files
- invader-info: Added `-T stub-count`
- Added slower but lower memory functions for decompression
- invader-build: Added `--compress` which compresses the cache file using level
  19 compression; this uses `-c` which `--forge-crc` now uses `-C`

### Changed
- To prevent people from confusing arguments of different programs, two of the
  short argument letters were changed:
    - invader-bitmap: Changed `-m` to `-M` to avoid confusion with `maps`
    - invader-build: Changed `-R` to `-N` to avoid confusion with `retail`
- invader-build: 32 byte tag strings are now zeroed out before copying
- invader-compress: Names and build strings are zeroed out before copying
- invader-build: The number of stubbed tags is now shown next to the tag count
- invader-info: The number of stubbed tags is now shown next to the tag count
- invader-compress: Levels higher than 19 are no longer allowed, as they take
  longer to decompress and require significantly more RAM
- invader-compress: Now exits more gracefully on failure
- invader-bitmap: Changed the default bump height to 0.026
- invader-compress: The default compression is now 19
- invader-build: The ting sound tag is now determined by the globals tag
  instead of its tag path when setting the gain based on engine version
- zstd's source code is no longer included in the repository

### Fixed
- invader-font: Fixed `-i` being used as both info and font size. It is now
  `-z` as expected.
- invader-bitmap: Fixed `-P` not working with non-.tif files
- invader-build: Fixed indexed sound tag data taking up more space than needed;
  this should result in a small file size reduction for any Halo Custom Edition
  maps that have indexed sounds
- Addressed some linking errors when building with MinGW

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
- New tool: invader-info - Displays an overview of a map file but can also show
  a specific value (`-T`): compressed, crc32, dirty, engine, map-type,
  scenario, scenario-path, tag-count, tags
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
- New tool: invader-compress - Compresses cache files using the Zstandard
  algorithm
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

[Mozzarilla]: https://github.com/MosesofEgypt/mozzarilla

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
- invader-crc: CRC32 can no longer be forged using this tool; use invader-build
  instead

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
- New tool: invader-archive - Archives all tags needed to build a map
- New tool: invader-bitmap - Creates bitmap tags (only supports 2D textures
  without color plate data but also has custom mipmap support)
- New tool: invader-build - Builds cache files
- New tool: invader-crc - Displays and spoofs CRC32 checksums of cache files;
  this is useful for using modified multiplayer maps
- New tool: invader-dependency - Outputs a list of tags that depend on a given
  tag
- New tool: invader-indexer - Outputs the list of tags in a cache file to a
  text file to be used with Invader
- New tool: invader-resource - Builds resource map files

[0.2.0]: https://github.com/SnowyMouse/invader/compare/0.1.0...0.2.0
[0.2.1]: https://github.com/SnowyMouse/invader/compare/0.2.0...0.2.1
[0.3.0]: https://github.com/SnowyMouse/invader/compare/0.2.1...0.3.0
[0.3.1]: https://github.com/SnowyMouse/invader/compare/0.3.0...0.3.1
[0.3.2]: https://github.com/SnowyMouse/invader/compare/0.3.1...0.3.2
[0.4.0]: https://github.com/SnowyMouse/invader/compare/0.3.2...0.4.0
[0.4.1]: https://github.com/SnowyMouse/invader/compare/0.4.0...0.4.1
[0.4.2]: https://github.com/SnowyMouse/invader/compare/0.4.1...0.4.2
[0.4.3]: https://github.com/SnowyMouse/invader/compare/0.4.2...0.4.3
[0.5.0]: https://github.com/SnowyMouse/invader/compare/0.4.3...0.5.0
[0.6.0]: https://github.com/SnowyMouse/invader/compare/0.5.0...0.6.0
[0.7.0]: https://github.com/SnowyMouse/invader/compare/0.6.0...0.7.0
[0.7.1]: https://github.com/SnowyMouse/invader/compare/0.7.0...0.7.1
[0.7.2]: https://github.com/SnowyMouse/invader/compare/0.7.1...0.7.2
[0.7.3]: https://github.com/SnowyMouse/invader/compare/0.7.2...0.7.3
[0.8.0]: https://github.com/SnowyMouse/invader/compare/0.7.3...0.8.0
[0.8.1]: https://github.com/SnowyMouse/invader/compare/0.8.0...0.8.1
[0.8.2]: https://github.com/SnowyMouse/invader/compare/0.8.1...0.8.2
[0.9.0]: https://github.com/SnowyMouse/invader/compare/0.8.2...0.9.0
[0.10.0]: https://github.com/SnowyMouse/invader/compare/0.9.0...0.10.0
[0.10.1]: https://github.com/SnowyMouse/invader/compare/0.10.0...0.10.1
[0.11.0]: https://github.com/SnowyMouse/invader/compare/0.10.1...0.11.0
[0.12.0]: https://github.com/SnowyMouse/invader/compare/0.11.0...0.12.0
[0.13.0]: https://github.com/SnowyMouse/invader/compare/0.12.0...0.13.0
[0.14.0]: https://github.com/SnowyMouse/invader/compare/0.13.0...0.14.0
[0.14.1]: https://github.com/SnowyMouse/invader/compare/0.14.0...0.14.1
[0.15.0]: https://github.com/SnowyMouse/invader/compare/0.14.1...0.15.0
[0.15.1]: https://github.com/SnowyMouse/invader/compare/0.15.0...0.15.1
[0.15.2]: https://github.com/SnowyMouse/invader/compare/0.15.1...0.15.2
[0.16.0]: https://github.com/SnowyMouse/invader/compare/0.15.2...0.16.0
[0.16.1]: https://github.com/SnowyMouse/invader/compare/0.16.0...0.16.1
[0.17.0]: https://github.com/SnowyMouse/invader/compare/0.16.1...0.17.0
[0.18.0]: https://github.com/SnowyMouse/invader/compare/0.17.0...0.18.0
[0.19.0]: https://github.com/SnowyMouse/invader/compare/0.18.0...0.19.0
[0.19.1]: https://github.com/SnowyMouse/invader/compare/0.19.0...0.19.1
[0.19.2]: https://github.com/SnowyMouse/invader/compare/0.19.1...0.19.2
[0.20.0]: https://github.com/SnowyMouse/invader/compare/0.19.2...0.20.0
[0.20.1]: https://github.com/SnowyMouse/invader/compare/0.20.0...0.20.1
[0.20.2]: https://github.com/SnowyMouse/invader/compare/0.20.1...0.20.2
[0.21.0]: https://github.com/SnowyMouse/invader/compare/0.20.2...0.21.0
[0.21.1]: https://github.com/SnowyMouse/invader/compare/0.21.0...0.21.1
[0.21.2]: https://github.com/SnowyMouse/invader/compare/0.21.1...0.21.2
[0.21.3]: https://github.com/SnowyMouse/invader/compare/0.21.2...0.21.3
[0.21.4]: https://github.com/SnowyMouse/invader/compare/0.21.3...0.21.4
[0.22.0]: https://github.com/SnowyMouse/invader/compare/0.21.4...0.22.0
[0.22.1]: https://github.com/SnowyMouse/invader/compare/0.22.0...0.22.1
[0.22.2]: https://github.com/SnowyMouse/invader/compare/0.22.1...0.22.2
[0.22.3]: https://github.com/SnowyMouse/invader/compare/0.22.2...0.22.3
[0.22.4]: https://github.com/SnowyMouse/invader/compare/0.22.3...0.22.4
[0.23.0]: https://github.com/SnowyMouse/invader/compare/0.22.4...0.23.0
[0.23.1]: https://github.com/SnowyMouse/invader/compare/0.23.0...0.23.1
[0.23.2]: https://github.com/SnowyMouse/invader/compare/0.23.1...0.23.2
[0.23.3]: https://github.com/SnowyMouse/invader/compare/0.23.2...0.23.3
[0.23.4]: https://github.com/SnowyMouse/invader/compare/0.23.3...0.23.4
[0.23.5]: https://github.com/SnowyMouse/invader/compare/0.23.4...0.23.5
[0.24.0]: https://github.com/SnowyMouse/invader/compare/0.23.5...0.24.0
[0.24.1]: https://github.com/SnowyMouse/invader/compare/0.24.0...0.24.1
[0.24.2]: https://github.com/SnowyMouse/invader/compare/0.24.1...0.24.2
[0.25.0]: https://github.com/SnowyMouse/invader/compare/0.24.2...0.25.0
[0.25.1]: https://github.com/SnowyMouse/invader/compare/0.25.0...0.25.1
[0.25.2]: https://github.com/SnowyMouse/invader/compare/0.25.1...0.25.2
[0.26.0]: https://github.com/SnowyMouse/invader/compare/0.25.2...0.26.0
[0.26.1]: https://github.com/SnowyMouse/invader/compare/0.26.0...0.26.1
[0.27.0]: https://github.com/SnowyMouse/invader/compare/0.26.1...0.27.0
[0.27.1]: https://github.com/SnowyMouse/invader/compare/0.27.0...0.27.1
[0.28.0]: https://github.com/SnowyMouse/invader/compare/0.27.1...0.28.0
[0.28.1]: https://github.com/SnowyMouse/invader/compare/0.28.0...0.28.1
[0.29.0]: https://github.com/SnowyMouse/invader/compare/0.28.1...0.29.0
[0.30.0]: https://github.com/SnowyMouse/invader/compare/0.29.0...0.30.0
[0.30.1]: https://github.com/SnowyMouse/invader/compare/0.30.0...0.30.1
[0.31.0]: https://github.com/SnowyMouse/invader/compare/0.30.1...0.31.0
[0.32.0]: https://github.com/SnowyMouse/invader/compare/0.31.0...0.32.0
[0.32.1]: https://github.com/SnowyMouse/invader/compare/0.32.0...0.32.1
[0.32.2]: https://github.com/SnowyMouse/invader/compare/0.32.1...0.32.2
[0.32.3]: https://github.com/SnowyMouse/invader/compare/0.32.2...0.32.3
[0.32.4]: https://github.com/SnowyMouse/invader/compare/0.32.3...0.32.4
[0.32.5]: https://github.com/SnowyMouse/invader/compare/0.32.4...0.32.5
[0.33.0]: https://github.com/SnowyMouse/invader/compare/0.32.5...0.33.0
[0.33.1]: https://github.com/SnowyMouse/invader/compare/0.33.0...0.33.1
[0.33.2]: https://github.com/SnowyMouse/invader/compare/0.33.1...0.33.2
[0.33.3]: https://github.com/SnowyMouse/invader/compare/0.33.2...0.33.3
[0.33.4]: https://github.com/SnowyMouse/invader/compare/0.33.3...0.33.4
[0.34.0]: https://github.com/SnowyMouse/invader/compare/0.33.4...0.34.0
[0.34.1]: https://github.com/SnowyMouse/invader/compare/0.34.0...0.34.1
[0.35.0]: https://github.com/SnowyMouse/invader/compare/0.34.1...0.35.0
[0.35.1]: https://github.com/SnowyMouse/invader/compare/0.35.0...0.35.1
[0.36.0]: https://github.com/SnowyMouse/invader/compare/0.35.1...0.36.0
[0.36.1]: https://github.com/SnowyMouse/invader/compare/0.36.0...0.36.1
[0.37.0]: https://github.com/SnowyMouse/invader/compare/0.36.1...0.37.0
[0.37.1]: https://github.com/SnowyMouse/invader/compare/0.37.0...0.37.1
[0.38.0]: https://github.com/SnowyMouse/invader/compare/0.37.1...0.38.0
[0.38.1]: https://github.com/SnowyMouse/invader/compare/0.38.0...0.38.1
[0.39.0]: https://github.com/SnowyMouse/invader/compare/0.38.1...0.39.0
[0.40.0]: https://github.com/SnowyMouse/invader/compare/0.39.0...0.40.0
[0.40.1]: https://github.com/SnowyMouse/invader/compare/0.40.0...0.40.1
[0.41.0]: https://github.com/SnowyMouse/invader/compare/0.40.1...0.41.0
[0.41.1]: https://github.com/SnowyMouse/invader/compare/0.41.0...0.41.1
[0.41.2]: https://github.com/SnowyMouse/invader/compare/0.41.1...0.41.2
[0.41.3]: https://github.com/SnowyMouse/invader/compare/0.41.2...0.41.3
[0.41.4]: https://github.com/SnowyMouse/invader/compare/0.41.3...0.41.4
[0.42.0]: https://github.com/SnowyMouse/invader/compare/0.41.4...0.42.0
[0.42.1]: https://github.com/SnowyMouse/invader/compare/0.42.0...0.42.1
[0.42.2]: https://github.com/SnowyMouse/invader/compare/0.42.1...0.42.2
[0.43.0]: https://github.com/SnowyMouse/invader/compare/0.42.2...0.43.0
[0.43.0]: https://github.com/SnowyMouse/invader/compare/0.43.0...0.44.0

[Untagged]: https://github.com/SnowyMouse/invader/compare/0.44.0...master
