# Invader
Invader is an open source toolkit for creating maps and assets for Halo: Combat
Evolved.

Our Discord server is https://discord.gg/RCX3nvw

Our IRC channel is #invader on [Libera Chat](https://libera.chat/)
(irc.libera.chat/6697 \[TLS\])

The official source code repository is https://github.com/SnowyMouse/invader

## License
See LICENSE.md for information about what is licensed under what.

## Getting started
This readme addresses a few topics:
- [Staying up-to-date]
- [Contributing]
- [Getting Invader]
- [Programs]
- [Frequently asked questions]

## Staying up-to-date
To check if you are on an up-to-date version, run one of the tools (e.g.
invader-info) with the `-i` parameter. On the top of the output is the version
number that corresponds to the installation of Invader (e.g.
`Invader 0.26.1.r1776.9db4cc5`). This is the project name (`Invader`), version
(`0.26.1`), commit number (`r1776`), and a truncated commit hash (`9db4cc5`).

Next, go to the [commits] and check the commit hash against the topmost commit.
If the commit hash in Invader matches the beginning of the full hash, then you
are using the latest build of Invader.

[commits]: https://github.com/SnowyMouse/invader/commits/master

Invader is not finished, so it is important to stay up-to-date to obtain the
latest features and fixes. Staying on an old version means staying on unfinished
software that likely has known issues.

## Contributing
See CONTRIBUTING.md.

## Getting Invader
Invader can be obtained by either downloading pre-compiled binaries or
compiling from source.

### Nightly builds (Windows)
You can download precompiled [Nightly Builds]. These will usually be up-to-date
unless commits were made very recently.

Note that these builds are Windows-only. So, if you are not on Windows, you
should not use these builds.

[Nightly Builds]: https://invader.opencarnage.net/builds/nightly/download-latest.html

### Building Invader
If you got this readme from an archive containing pre-compiled Invader
binaries, this section probably doesn't apply to you, but you are welcome to
compile Invader if you want to. Regardless, you can browse and download the
source code for free on [GitHub].

[GitHub]: https://github.com/SnowyMouse/invader

If you use Arch Linux, the [Arch Linux AUR] has a package you can use to build
Invader. This will pull from the main repo on GitHub, so you only have to
rebuild the package when you want to update.

[Arch Linux AUR]: https://aur.archlinux.org/packages/invader-git/

#### Dependencies
Invader depends on software in order for it to build and work properly. This
section lists the dependencies required to fully utilize Invader. Note that
some of these dependencies may have their own dependencies (and so on), but if
you use a package manager to get them, then it should take care of that for you.

##### Required dependencies
- C++17 compiler
- C11 compiler
- CMake 3.12 or newer
- Python 3.7 or newer
- LibTIFF 3.6 or newer
- libvorbis 1.3.6 or newer
- libsamplerate 0.1.9 or newer
- Qt6
- SDL2
- zlib

##### Optional dependencies
- LibArchive ([invader-archive])
- freetype ([invader-font])
- git (git commit hash in version - build only)

#### Compiling (POSIX)
First, you will need to download the Invader repository onto your computer. You
can do this using the command:

```
git clone https://github.com/SnowyMouse/invader --recursive
```

Everything in this section, from this point on, assumes the Invader repository
was cloned in a directory called "invader" in the current directory.

Next, you will need to create an out-of-source build directory. You can use
this command to make the build directory and CD into it upon success:

```
mkdir invader-build && cd invader-build
```

Next, use the `cmake` command to set up your build directory, optionally
specifying the build type to Release:

```
cmake ../invader -DCMAKE_BUILD_TYPE=Release
```

Lastly, you can compile this using the `make` command.

```
make
```

## Programs
To remove the reliance of one huge executable, something that has caused issues
with Halo Custom Edition's tool.exe, as well as make things easier to develop,
this project is split into different programs.
- [invader-archive]
- [invader-bitmap]
- [invader-bludgeon]
- [invader-build]
- [invader-collection]
- [invader-compare]
- [invader-convert]
- [invader-dependency]
- [invader-edit]
- [invader-edit-qt]
- [invader-extract]
- [invader-font]
- [invader-index]
- [invader-info]
- [invader-model]
- [invader-recover]
- [invader-refactor]
- [invader-resource]
- [invader-script]
- [invader-sound]
- [invader-string]
- [invader-strip]

### invader-archive
This program generates a .tar.xz archive containing all of the tags used to
build a map.

```
Usage: invader-archive [options] <-g <engine> <scenario> | -s tag.class>

Generate .tar.xz archives of the tags required to build a cache file.

Options:
  -C --copy                    Copy instead of making an archive.
  -e --exclude <dir>           Exclude copying any tags that share a path with
                               a tag in specified directory. Use multiple times
                               to exclude multiple directories.
  -E --exclude-matched         Exclude copying any tags that are also located
                               in the specified directory and are functionally
                               the same. Use multiple times to exclude multiple
                               directories.
  -F --format <format>         Specify format. Valid formats are: 7z, tar-gz,
                               tar-xz, tar-zst, zip. Default format is 7z
  -g --game-engine <engine>    Specify the game engine. Valid engines are:
                               gbx-custom, gbx-demo, gbx-retail, mcc-cea,
                               native, xbox-demo, xbox-ntsc, xbox-ntsc-jp,
                               xbox-ntsc-tw, xbox-pal
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -o --output <file>           Output to a specific file. Extension must be
                               .tar.xz unless using --copy which then it's a
                               directory.
  -O --overwrite               Overwrite tags if they already exist if using
                               --copy
  -P --fs-path                 Use a filesystem path for the tag.
  -s --single-tag              Archive a tag tree instead of a cache file.
  -t --tags <dir>              Add the specified tags directory. Use multiple
                               times to add more directories, ordered by
                               precedence. Default (if unset): "tags"
  -v --verbose                 Print whether or not tags are omitted. Do
                               verbose comparisons.
```

### invader-bitmap
This program generates bitmap tags from images. For source images, .tif, .tiff,
.png, .tga, and .bmp extensions are supported.

Note that image sharpening and blurring, while supported, are not recommended,
and output may not exactly match the Halo Editing Kit's output.

```
Usage: invader-bitmap [options] <bitmap-tag>

Create or modify a bitmap tag.

Options:
  -A --alpha-bias <bias>       Set the alpha bias from -1.0 to 1.0. Default
                               (new tag): 0.0
  -B --budget <length>         Set the maximum length of a sprite sheet. Can be
                               32, 64, 128, 256, 512, or 1024. Default (new
                               tag): 32
  -C --budget-count <count>    Multiply the maximum length squared to set the
                               maximum number of pixels. Setting this to 0
                               disables budgeting. Default (new tag): 0
  -d --data <dir>              Use the specified data directory. Default:
                               "data"
  -D --dithering <val>         Apply dithering to 16-bit or p8 bitmaps. Can be:
                               off or on. Default (new tag): off
  -f --detail-fade <factor>    Set detail fade factor. Default (new tag): 0.0
  -F --format <type>           Pixel format. Can be: 32-bit, 16-bit,
                               monochrome, dxt5, dxt3, dxt1, or auto. 'auto'
                               will be replaced with the best lossless format.
                               Default (new tag): auto
  -h --help                    Show this list of options.
  -H --bump-height <height>    Set the apparent bumpmap height from 0.0 to 1.0.
                               Default (new tag): 0.026
  -i --info                    Show credits, source info, and other info.
  -I --ignore-tag              Ignore the tag data if the tag exists.
  -M --mipmap-count <count>    Set maximum mipmaps. Default (new tag): 32767
  -n --allow-non-power-of-two  Allow color plates with non-power-of-two,
                               non-interface bitmaps.
  -p --bump-palettize <val>    Set the bumpmap palettization setting. Can be:
                               off or on. Default (new tag): off
  -P --fs-path                 Use a filesystem path for the tag.
  -r --reg-point-hack <val>    Ignore sequence borders when calculating
                               registration point (AKA 'filthy sprite bug
                               fix'). Can be: off or on. Default (new tag): off
  -R --regenerate              Use the bitmap tag's compressed color plate data
                               as data.
  -s --mipmap-scale <type>     Mipmap scale type. This does not save in .bitmap
                               tags. Can be: linear, nearest_alpha, nearest.
                               Default (new tag): linear
  -S --square-sheets           Force square sprite sheets (works around
                               particles being incorrectly stretched).
  -t --tags <dir>              Use the specified tags directory. Default:
                               "tags"
  -T --type <type>             Set the type of bitmap. Can be: 2d_textures,
                               3d_textures, cube_maps, interface_bitmaps, or
                               sprites. Default (new tag): 2d_textures
  -u --usage <usage>           Set the bitmap usage. Can be: alpha_blend,
                               default, height_map, detail_map, light_map,
                               vector_map. Default: default
```

Refer to [Creating a bitmap] for information on how to create bitmap tags.

[Creating a bitmap]: https://github.com/SnowyMouse/invader/wiki/Creating-a-bitmap

### invader-bludgeon
This program convinces broken tags to work with Invader.

```
Usage: invader-bludgeon [options] <-b [expr] | <tag>>

Convinces tags to work with Invader.

Options:
  -b --batch <expr>            Run the command on all tags with a given
                               expression.
  -e --batch-exclude <expr>    Run the command on all tags that do not match a
                               given expression. This takes precedence over
                               --batch
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -j --threads                 Set the number of threads to use for parallel
                               bludgeoning when using --all. Default: CPU
                               thread count
  -P --fs-path                 Use a filesystem path for the tag.
  -t --tags <dir>              Use the specified tags directory. Default:
                               "tags"
  -T --type                    Type of bludgeoning. Can be:
                               broken-lens-flare-function-scale,
                               incorrect-sound-buffer, invalid-enums,
                               invalid-indices, invalid-strings,
                               invalid-model-markers,
                               invalid-reference-classes,
                               invalid-uppercase-references,
                               mismatched-sound-enums, missing-script-source,
                               missing-vertices, nonnormal-vectors,
                               out-of-range, everything
```

### invader-build
This program builds cache files.

```
Usage: invader-build [options] -g <target> <scenario>

Build a cache file.

Options:
  -a --anniversary-mode        Enable anniversary graphics and audio (CEA only)
  -A --auto-forge              Ensure the map will be network compatible with
                               the target engine's stock maps.
  -b --stock-resource-bounds   Only index tags if the tag's index is within
                               stock Custom Edition's resource map bounds.
                               (Custom Edition only)
  -B --build-string <ver>      Set the build string in the header.
  -C --forge-crc <crc>         Forge the CRC32 value of the map after building
                               it.
  -E --extend-file-limits      Extend file size limits to 2 GiB regardless of
                               if the target engine will support the cache
                               file.
  -g --game-engine <engine>    Specify the game engine. Valid engines are:
                               gbx-custom, gbx-demo, gbx-retail, mcc-cea,
                               native, xbox-demo, xbox-ntsc, xbox-ntsc-jp,
                               xbox-ntsc-tw, xbox-pal
  -h --help                    Show this list of options.
  -H --hide-pedantic-warnings  Don't show minor warnings.
  -i --info                    Show credits, source info, and other info.
  -l --level <level>           Set the compression level (Xbox maps only). Must
                               be between 0 and 9. Default: 9
  -m --maps <dir>              Use the specified maps directory. Default:
                               "maps"
  -N --rename-scenario <name>  Rename the scenario.
  -o --output <file>           Output to a specific file.
  -O --optimize                Optimize tag space. This will drastically
                               increase the amount of time required to build
                               the cache file.
  -P --fs-path                 Use a filesystem path for the tag.
  -q --quiet                   Only output error messages.
  -r --resource-usage <usage>  Specify the behavior for using resource maps.
                               Must be: none (don't use resource maps), check
                               (check resource maps), always (always index tags
                               in resource maps - Custom Edition only).
                               Default: none
  -R --resource-maps <dir>     Specify the directory for loading resource maps.
                               (by default this is the maps directory)
  -t --tags <dir>              Add the specified tags directory. Use multiple
                               times to add more directories, ordered by
                               precedence. Default (if unset): "tags"
  -T --tag-space <size>        Override the tag space. This may result in a map
                               that does not work with the stock games. You can
                               specify the number of bytes, optionally
                               suffixing with K (for KiB) or M (for MiB), or
                               specify in hexadecimal the number of bytes (e.g.
                               0x1000).
  -w --with-index <file>       Use an index file for the tags, ensuring the
                               map's tags are ordered in the same way.
```

#### Tag patches
In some instances, specific tags will be modified. Some of these are a holdover
from tool.exe, or they are done to account for different versions of the game.
Admittedly, this is poor design, but that's ultimately on Gearbox and Bungie for
changing the tags in the first place rather than creating new tags.

Invader will silently modify a few tags for single player:

- `weapons\pistol\pistol.weapon`
    - Minimum error: 0.2°
    - Error angle: 0.2° to 0.4°
- `weapons\pistol\bullet.damage_effect`
    - Elite energy shield modifier: 0.8
- `weapons\plasma rifle\plasma rifle.weapon`
    - Error angle: 0.25° to 2.5°

This is a holdover from tool.exe, and it is done in Invader to prevent the core
single player experience from being affected. If you do not want it to do this,
copy these respective tags to a different path.

Invader will also silently modify the ting sound effect for multiplayer to have
a gain of 1.0 if Custom Edition and 0.2 if not. This is so the sound is not too
loud or too quiet when played on their respective versions.

Invader will modify a few tags when building a stock multiplayer map:

- `vehicles\ghost\ghost bolt.damage_effect`,
  `vehicles\banshee\banshee bolt.damage_effect`
    - Stun: 0.0 if Custom Edition, else 1.0
    - Maximum stun: 0.0 if Custom Edition, else 1.0
    - Stun time: 0.0 if Custom Edition, else 0.15
- `vehicles\rwarthog\rwarthog_gun.weapon`
    - Autoaim angle: 6.0° if Custom Edition, else 1.0°
    - Deviation angle: 12.0° if Custom Edition, else 1.0°

This is done to prevent desyncing caused by using tag sets from different
versions of the game. If you do not want it to do this, copy these respective
tags to a different path. Custom maps (i.e. maps that don't use stock scenario
tag names) are not affected by this. Also, if any changes are applied, a minor
warning will be emitted.

### invader-collection
This program creates .tag_collection tag files from .txt files, where each path
is on a separate line.

```
Usage: invader-collection [options] <tag>

Generate tag_collection tags.

Options:
  -d --data <dir>              Use the specified data directory. Default:
                               "data"
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -P --fs-path                 Use a filesystem path for the tag.
  -t --tags <dir>              Use the specified tags directory. Default:
                               "tags"
```

### invader-compare
This program compares tags against maps, maps against maps, and tags against
tags.

```
Usage: invader-compare [options] <-I <opts>> <-I <opts>> [<-I <opts>> ...]

Compare tags against other tags.

Options:
  -a --all                     Only match if tags are in all inputs.
  -B --by-path <path-type>     Set what tags get compared against other tags.
                               By default, only tags with the same relative
                               path are checked. Using "any" ignores paths
                               completely (useful for finding duplicates when
                               both inputs are different) while "different"
                               only checks tags with different paths (useful
                               for finding duplicates when both inputs are the
                               same). Can be: any, different, or same (default)
  -e --search-exclude <expr>   Search for tags (* and ? are wildcards) and
                               ignore these. Use multiple times for multiple
                               queries. This takes precedence over --search.
  -f --functional              Precompile the tags before comparison to check
                               for only functional differences.
  -G --ignore-resources        Ignore resource maps for the current map input.
                               This option must be used after --input.
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -I --input                   Add an input. This option is required before
                               using --tags, --maps, --map, and
                               --ignore-resources.
  -j --threads                 Set the number of threads to use for comparison.
                               Default: 1
  -m --maps                    Add a maps directory to the input to specify
                               where to find resource files for a map. This
                               option must be used after --input.
  -M --map                     Add a map to the input. Only one map can be
                               specified per input. If a maps directory isn't
                               specified, then the map's directory will be
                               used. This option must be used after --input.
  -p --precision               Allow for slight differences in floats to
                               account for precision loss.
  -s --search <expr>           Search for tags (* and ? are wildcards) and
                               compare these. Use multiple times for multiple
                               queries. If unspecified, all tags will be
                               compared.
  -S --show                    Can be: all, matched, or mismatched. Default:
                               all
  -t --tags                    Add a tags directory to the input. Specify
                               multiple tag directories in order of precedence
                               for the input. This option must be used after
                               --input.
  -v --verbose                 Output more information on the differences
                               between tags to standard output. This will not
                               work with --functional.
```

### invader-convert
This program converts tags from one type to another. This is especially useful
in conjunction with [invader-refactor] for porting maps between different
versions of the game.

```
Usage: invader-convert [options] <--all | -s <tag>>

Convert from one tag type to another.

Options:
  -a --all                     Convert all tags. This cannot be used with -s.
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -o --output-tags <dir>       Set the output tags directory.
  -O --overwrite               Overwrite any output tags if they exist.
  -P --fs-path                 Use a filesystem path for the tag.
  -s --single-tag              Convert a specific tag. This can be specified
                               multiple times for multiple tags, but cannot be
                               used with -a.
  -t --tags <dir>              Use the specified tags directory. Default:
                               "tags"
  -T --type <type>             Type of conversion. Can be: gbxmodel-to-model
                               (g2m), model-to-gbxmodel (m2g),
                               chicago-extended-to-chicago (x2c)
```

### invader-dependency
This program finds tags that directly depend on a given tag.

```
Usage: invader-dependency [options] <tag.class>

Check dependencies for a tag.

Options:
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -P --fs-path                 Use a filesystem path for the tag.
  -r --recursive               Recursively get all depended tags.
  -R --reverse                 Find all tags that depend on the tag, instead.
  -t --tags <dir>              Add the specified tags directory. Use multiple
                               times to add more directories, ordered by
                               precedence. Default (if unset): "tags"
```

### invader-edit
This program edits tags in a command-line interface. This is primarily used for
scripting. If you want to edit tags in a graphical user interface, use
[invader-edit-qt].

```
Usage: invader-edit [options] <tag.class>

Edit tags via command-line.

Options:
  -c --copy <key> <pos>        Copy the selected struct(s) to the given index
                               or "end" if the end of the array.
  -C --count <key>             Get the number of elements in the array at the
                               given key.
  -E --erase <key>             Delete the selected struct(s).
  -G --get <key>               Get the value with the given key.
  -h --help                    Show this list of options.
  -H --checksum                Output the calculated checksum of the tag.
  -i --info                    Show credits, source info, and other info.
  -I --insert <key> <#> <pos>  Add # structs to the given index or "end" if the
                               end of the array.
  -l --list                    List all elements in a tag.
  -L --list-values             List all elements and values in a tag. This may
                               be slow on large tags.
  -M --move <key> <pos>        Swap the selected structs with the structs at
                               the given index or "end" if the end of the
                               array. The regions must not intersect.
  -n --no-safeguards           Allow all tag data to be edited (proceed at your
                               own risk)
  -N --new                     Create a new tag
  -o --output <file>           Output the tag to a different path rather than
                               overwriting it.
  -O --save-as <tag>           Output the tag to a different path relative to
                               the tags directory rather than overwriting it.
  -P --fs-path                 Use a filesystem path for the tag.
  -S --set <key> <val>         Set the value at the given key to the given
                               value.
  -t --tags <dir>              Use the specified tags directory. Default:
                               "tags"
  -V --verify-checksum         Verify that the checksum in the header is
                               correct and print the result.
```

### invader-edit-qt
This program edits tags in a Qt-based GUI.

```
Usage: invader-edit-qt [options] [<tag1> [tag2] [...]]

Edit tag files.

Options:
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -L --listing-mode            Set the listing behavior. Can be: fast,
                               recursive (default: fast)
  -n --no-safeguards           Allow all tag data to be edited (proceed at your
                               own risk)
  -P --fs-path                 Use a filesystem path for the tag.
  -t --tags <dir>              Add the specified tags directory. Use multiple
                               times to add more directories, ordered by
                               precedence. Default (if unset): "tags"
```

### invader-extract
This program extracts tags from cache files.

```
Usage: invader-extract [options] <map>

Extract data from cache files.

Options:
  -e --search-exclude <expr>   Search for tags (* and ? are wildcards) and
                               ignore these. Use multiple times for multiple
                               queries. This takes precedence over --search.
  -G --ignore-resources        Ignore resource maps.
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -m --maps <dir>              Use the specified maps directory. Default:
                               "maps"
  -n --non-mp-globals          Enable extraction of non-multiplayer .globals
  -O --overwrite               Overwrite tags if they already exist
  -P --fs-path                 Use a filesystem path for the tag.
  -r --recursive               Extract tag dependencies
  -s --search <expr>           Search for tags (* and ? are wildcards) and
                               extract these. Use multiple times for multiple
                               queries. If unspecified, all tags will be
                               extracted.
  -t --tags <dir>              Use the specified tags directory. Default:
                               "tags"
```

### invader-font
This program generates font tags.

```
Usage: invader-font [options] <font-tag>

Create font tags from OTF/TTF files.

Options:
  -d --data <dir>              Use the specified data directory. Default:
                               "data"
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -l --latin1                  Use 256 characters only (smaller)
  -P --fs-path                 Use a filesystem path for the tag.
  -s --font-size <px>          Set the font size in pixels.
  -t --tags <dir>              Use the specified tags directory. Default:
                               "tags"
```

### invader-index
This program builds index files for usage with `--with-index` with
invader-build.

```
Usage: invader-index [options] <input-map> <output-txt>

Create a file listing the tags of a map.

Options:
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
```

### invader-info
This program displays metadata of a cache file.

```
Usage: invader-info [option] <map>

Display map metadata.

Options:
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -T --type <type>             Set the type of data to show. Can be overview
                               (default), build, compression_ratio, crc32,
                               crc32_mismatched, engine, external_bitmaps,
                               external_bitmaps_count, external_bitmap_indices,
                               external_bitmap_indices_count,
                               external_bitmap_pointers,
                               external_bitmap_pointers_count,
                               external_indices, external_indices_count,
                               external_loc_indices,
                               external_loc_indices_count, external_sounds,
                               external_sounds_count, external_sound_indices,
                               external_sound_indices_count,
                               external_sound_pointers,
                               external_sound_pointers_count, external_tags,
                               external_tags_count, internal_bitmaps,
                               internal_bitmaps_count, internal_sounds,
                               internal_sounds_count, is_compressed, is_dirty,
                               is_protected, languages, map_type,
                               protection_issues, scenario, scenario_path,
                               stub_count, tag_order_match, tags, tags_count,
                               uncompressed_size, uses_external_pointers
```

### invader-model
This program compiles model tags. It can compile both model and gbxmodel formats
directly from .JMS files.

JMS files should be placed in the data folder in a `models` folder in a folder
relative to where the model tag will be generated in the tags directory. So, if
you want to compile a model `tags/weapons/pistol/pistol.model`, you would put
your JMS files in `data/weapons/pistol/models`. A tutorial will be made sometime
in the future regarding this.

You can make JMS files for free using [Halo-Asset-Blender-Development-Toolset]
in [Blender].

[Halo-Asset-Blender-Development-Toolset]: https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
[Blender]: https://www.blender.org/

```
Usage: invader-model [options] <model-tag>

Compile a model tag.

Options:
  -d --data <dir>              Use the specified data directory. Default:
                               "data"
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -P --fs-path                 Use a filesystem path for the tag.
  -t --tags <dir>              Add the specified tags directory. Use multiple
                               times to add more directories, ordered by
                               precedence. Default (if unset): "tags"
  -T --type <type>             Specify the type of model. Can be: model,
                               gbxmodel
```

### invader-recover
This program recovers source data from bitmaps (if color plate data is present),
models, string lists, tag collections, and scenario scripts.

```
Usage: invader-recover [options] <-b <expr> | <tag.group>>

Recover source data from tags.

Options:
  -b --batch <expr>            Run the command on all tags with a given
                               expression.
  -d --data <dir>              Use the specified data directory. Default:
                               "data"
  -e --batch-exclude <expr>    Run the command on all tags that do not match a
                               given expression. This takes precedence over
                               --batch
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -O --overwrite               Overwrite data if it already exists
  -P --fs-path                 Use a filesystem path for the tag.
  -t --tags <dir>              Use the specified tags directory. Default:
                               "tags"
```

### invader-refactor
This program renames and moves tag references.

```
Usage: invader-refactor <-M <mode>> [options]

Find and replace tag references.

Options:
  -D --dry-run                 Do not actually make any changes. This is useful
                               for checking for errors before committing
                               anything, although filesystem errors may not be
                               caught.
  -g --group <f> <t>           Refactor all tags of a given group to another
                               group. All tags in the destination group must
                               exist. This can be specified multiple times but
                               cannot be used with --recursive or -M move.
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -M --mode <mode>             Specify what to do with the file if it exists.
                               If using move, then the tag is moved (the tag
                               must exist on the filesystem) while also
                               changing all references to the tag to the new
                               path. If using no-move, then the tag is not
                               moved (the destination tag must exist on the
                               filesystem unless you use --unsafe) while also
                               changing all references to the tag to the new
                               path. If using copy, then the tag is copied (the
                               tag must exist on the filesystem) and references
                               to the tag are not changed except for other tags
                               copied by this command. Can be: copy, move,
                               no-move
  -r --recursive <f> <t>       Recursively move all tags in a directory. This
                               will fail if a tag is present in both the old
                               and new directories, it cannot be used with
                               no-move. This can only be specified once per
                               operation and cannot be used with --tag.
  -R --replace-string <a> <b>  Replaces all instances in a path of <a> with
                               <b>. This can be used multiple times for
                               multiple replacements. If --group or --recursive
                               are used, this applies to the output of those.
                               Otherwise, it applies to all tags.
  -s --single-tag <path>       Make changes to a single tag, only, rather than
                               the whole tags directory.
  -t --tags <dir>              Add the specified tags directory. Use multiple
                               times to add more directories, ordered by
                               precedence. Default (if unset): "tags"
  -T --tag <f> <t>             Refactor an individual tag. This can be
                               specified multiple times but cannot be used with
                               --recursive.
  -U --unsafe                  Do not require the destination tags to exist if
                               using no-move
```

### invader-resource
This program builds resource maps. Custom Edition resource maps require all
stock tags to be present in order to be built. Files created by this tool are
not guaranteed to work with existing cache files.

```
Usage: invader-resource [options] -g <engine> -T <type>

Create resource maps.

Options:
  -c --concatenate <file>      Concatenate against the resource map at a path.
                               This cannot be used with -T loc
  -g --game-engine <engine>    Specify the game engine. Demo and retail maps
                               also require either --with-index or --with-map
                               to be specified at least once. Valid engines
                               are: gbx-custom, gbx-demo, gbx-retail, mcc-cea.
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -m --maps <dir>              Use the specified maps directory. Default:
                               "maps"
  -M --with-map <file>         Use a map file for the tags. This can be
                               specified multiple times.
  -S --show-matched            Print the paths of any matched tags found when
                               using --concatenate.
  -t --tags <dir>              Add the specified tags directory. Use multiple
                               times to add more directories, ordered by
                               precedence. Default (if unset): "tags"
  -T --type <type>             Set the resource map. Can be: bitmaps, sounds,
                               or loc.
  -w --with-index <file>       Use an index file for the tags, ensuring tags
                               are ordered in the same way (barring
                               duplicates).
```

### invader-script
This program compiles .hsc scripts. Scripts are stored in the `scripts` folder
relative to the scenario tag's path in the data directory.

It uses the [Rat In a Tube](https://github.com/SnowyMouse/riat) script compiler.

```
Usage: invader-script [options] <scenario>

Compile scripts. Unless otherwise specified, global scripts are always compiled.

Options:
  -c --clear                   Clear all script data from the scenario tag
  -d --data <dir>              Use the specified data directory. Default:
                               "data"
  -e --explicit <source>       Explicitly compile the given source in the
                               script directory. This argument can be used
                               multiple times.
  -E --exclude-global-scripts  Do not use global_scripts source.
  -g --game-engine <engine>    Specify the game engine. Valid engines are:
                               gbx-custom, gbx-demo, gbx-retail, mcc-cea,
                               native, xbox-demo, xbox-ntsc, xbox-ntsc-jp,
                               xbox-ntsc-tw, xbox-pal
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -P --fs-path                 Use a filesystem path for the tag.
  -r --reload-scripts          Only recompile sources referenced by the tag.
  -R --regenerate              Use the scenario tag's script source data as
                               data.
  -t --tags <dir>              Add the specified tags directory. Use multiple
                               times to add more directories, ordered by
                               precedence. Default (if unset): "tags"
```


### invader-sound
This program generates sound tags. Sound tag data is stored in the data
directory as a directory containing .wav and/or .flac files. Each file is one
permutation, and only 16-bit and 24-bit PCM with either one or two channels are
supported as input.

You cannot have two permutations with the same name (i.e. mypermutation.wav and
mypermutation.flac). Also, unless you are modifying an existing sound tag, you
will need to supply a sound class.

You can, however, supply permutations with differing bit depths and sample
rates and channel count. By default, the highest sample rate will be used for
the entire tag, and if that is not 22050 Hz or 44100 Hz, then it will
automatically be resampled.

```
Usage: invader-sound [options] <sound-tag>

Create or modify a sound tag.

Options:
  -b --bitrate <br>            Set the bitrate in kilobits per second. This
                               only applies to vorbis.
  -c --class <class>           Set the class. This is required when generating
                               new sounds. Can be: ambient_computers,
                               ambient_machinery, ambient_nature,
                               device_computers, device_door,
                               device_force_field, device_machinery,
                               device_nature, first_person_damage, game_event,
                               music, object_impacts, particle_impacts,
                               projectile_impact, projectile_detonation,
                               scripted_dialog_force_unspatialized,
                               scripted_dialog_other, scripted_dialog_player,
                               scripted_effect, slow_particle_impacts,
                               unit_dialog, unit_footsteps, vehicle_collision,
                               vehicle_engine, weapon_charge, weapon_empty,
                               weapon_fire, weapon_idle, weapon_overheat,
                               weapon_ready, weapon_reload
  -C --channel-count <#>       Set the channel count. Can be: mono, stereo. By
                               default, this is determined based on the input
                               audio.
  -d --data <dir>              Use the specified data directory. Default:
                               "data"
  -F --format <fmt>            Set the format. Can be: 16-bit_pcm, ogg_vorbis,
                               or xbox_adpcm. Default: 16-bit_pcm
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -j --threads                 Set the number of threads to use for parallel
                               resampling and encoding. Default: CPU thread
                               count
  -l --compress-level <lvl>    Set the compression level. This can be between
                               0.0 and 1.0. For Ogg Vorbis, higher levels
                               result in better quality but worse sizes.
                               Default: 0.8
  -P --fs-path                 Use a filesystem path for the tag.
  -r --sample-rate <Hz>        Set the sample rate in Hz. Halo supports 22050
                               and 44100. By default, this is determined based
                               on the input audio.
  -s --split                   Split permutations into 227.5 KiB chunks. This
                               is necessary for longer sounds (e.g. music) when
                               being played in the original Halo engine.
  -S --no-split                Do not split permutations.
  -t --tags <dir>              Use the specified tags directory. Default:
                               "tags"
```

Refer to [Creating a sound] for a guide on how to create sound tags.

[Creating a sound]: https://github.com/SnowyMouse/invader/wiki/Creating-a-sound

### invader-string
This program generates string tags. If building a unicode or latin-1 tag,
strings are stored in a .txt file, with each string ending with a line,
`###END-STRING###`.

```
Usage: invader-string [options] <tag>

Generate string list tags.

Options:
  -d --data <dir>              Use the specified data directory. Default:
                               "data"
  -f --format                  Set string list format. Can be unicode or
                               latin-1. Must be specified if a string tag is
                               not present.
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -P --fs-path                 Use a filesystem path for the tag.
  -t --tags <dir>              Use the specified tags directory. Default:
                               "tags"
```

### invader-strip
This program strips hidden data from tags and recalculates their CRC32 values.

```
Usage: invader-strip [options] <-b [expr] | <tag>>

Strips extra hidden data from tags.

Options:
  -b --batch <expr>            Run the command on all tags with a given
                               expression.
  -e --batch-exclude <expr>    Run the command on all tags that do not match a
                               given expression. This takes precedence over
                               --batch
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -P --fs-path                 Use a filesystem path for the tag.
  -t --tags <dir>              Use the specified tags directory. Default:
                               "tags"
```

## Frequently asked questions
These are a selection of questions that have been asked over the course of
Invader's development.
- [I get errors when building HEK tags or tags extracted with invader-extract]
- [What operating systems are supported?]
- [Who owns my map when I build it?]
- [Why GPL and not MIT or BSD?]
- [Why is Invader multiple programs?]
- [Are there any GUI tools?]
- [Are 32-bit Windows builds available?]
- [Can invader-build create .yelo maps?]
- [Can invader-build create Xbox maps?]
- [The HEK says my bitmap tag is "too large" when opening.]
- [How close to completion is Invader?]
- [Should I use invader-build for my map right now?]

### I get errors when building HEK tags or tags extracted with invader-extract
The stock Halo Editing Kit tags as well as a number of extracted tags have a
number of issues such as invalid enumerators, indices, and references. In many
cases, this can result in undefined, unexpected behavior or crashing. Obviously,
the stock Halo Editing Kit doesn't crash, but this data is still invalid
nevertheless.

The most common issues can be fixed by running the following command:

```
invader-bludgeon -b "*" -T invalid-indices -T invalid-enums -T out-of-range
```

The stock Halo Editing Kit also comes with a number of tags that reference model
tags instead of gbxmodel. The Halo Editing Kit automatically changes these on
tag load, likely because Gearbox developers felt it would be better to just do
this than fix their tags (since their engine only supports gbxmodel tags).

Invader supports multiple engines INCLUDING the Xbox version, so we cannot just
do this. Fortunately, there is a command you can run that will actually fix the
tags for you:

```
invader-refactor -M no-move -c model gbxmodel
```

Lastly, invader-build checks the CRC32 checksums of each tag. This is done to
ensure that the tags have been modified correctly, giving you a warning in the
case they have not. However, some HEK tags do not have this set for some reason.
You can fix this by running this command:

```
invader-strip -b "*"
```

Note that the above commands take `-t <path-to-tags-directory>` just like
invader-build, so if you use a tags directory that isn't "tags" on the current
directory, simply append that. And, of course, if Invader isn't installed
system-wide, you will need to replace the program names with paths to their
respective executables.

### What operating systems are supported?
Invader does not support any one operating system, but Invader is developed on
Arch Linux, and 64-bit Windows builds are available via [Nightly Builds].

[Nightly Builds]: https://invader.opencarnage.net/builds/nightly/download-latest.html

### Who owns my map when I build it?
There is a commonly-spread myth that, once you build a map with tool.exe, your
map belongs to Microsoft. Likewise, people may assume that, if you build a map
with Invader, your map belongs to us. However, this is like saying that, once
you print a document with an HP printer, your document belongs to HP. That's not
how it works.

The only "ownership" that applies to your map is to the tags and assets in the
map, as a map is effectively an archive of a tag directory intended to be loaded
by the game. You own your BSP, Microsoft owns the assault rifle, etc. Not one
person owns your map unless you made *all* of the tags to your map, in which
case *you* would certainly own it.

Note, however, that you may be subjected to additional license(s) if you choose
to use assets you don't own (assuming you even have permission to use them),
including the assets that came with the Halo Editing Kit. Using Invader does not
give you permission to use any of these assets, nor does it give you permission
to use any extracted assets (or to even extract those assets). Consult a legal
expert if you need assurance on your rights.

### Why GPL and not MIT or BSD?
This is primarily due to Halo's modding history. The Halo: CE modding community
has traditionally obfuscated knowledge that could be used to help other people.
This is often done through these means:
- [Map protection](http://forum.halomaps.org/index.cfm?page=topic&topicID=16885)
- [Writing closed source software](http://halo.isimaginary.com/)
- [Obfuscating software](http://forum.halomaps.org/index.cfm?page=topic&topicID=51196&start=112)
- [Keeping helpful information locked behind secret forums and groups](http://www.macgamingmods.com/forum/viewtopic.php?f=40&t=7948)

Many tools have been released without accompanying source code, and in nearly
all cases, they have gone unmaintained. Even if the idea of a source code
release is expressed, this rarely happens.

As time goes on, unmaintained, closed-source software becomes more seemingly
broken, incomplete, or even incompatible. For example, the Halo Editing Kit has
numerous issues that prevent people from making the content they want to make,
yet [it is still the only way to fully create maps].

[it is still the only way to fully create maps]: https://opencarnage.net/index.php?/topic/7765-replacing-the-halo-editing-kit-with-open-source-software/

Many great accomplishments have been lost to time, as most of the people who
made them are no longer there to support what they have made. Unfortunately,
most users do not make any sort of effort to replace this software with working
software as they either do not know how to or they do not feel it is worth the
time or effort. Therefore, the broken, unmaintained software continues being
used. This is where Invader comes in.

An editing kit made in the open ensures that learned information shall be
available for those who want to learn and expand upon or make their own tools.

Modding tools run by a user should be open for all to use, share, change, and
learn from. At the same time, we also feel that a requirement for people give
back to the community the source code to any derivatives of our tools they have
chosen to share is not too much to ask for.

This ensures that modding stays free and open instead of left to stagnation
behind closed groups and obfuscation as modding this game has been in the past.

Also, a weak license like MIT has proven to not be helpful. A common argument is
that more people will be willing to use and contribute to it. However, a number
of open source mods, which were released as MIT and BSD, have been used in
closed source projects, or they have been converted to closed source with the
[original open source versions receiving zero contributions].

[original open source versions receiving zero contributions]: https://github.com/Chaosvex/HAC2

That is why Invader uses version 3 of the GNU GPL. It keeps it free for everyone
to use.

### Why is Invader multiple programs?
Invader takes the approach of making each program do one thing well. There are a
few benefits to this:

Invader emphasizes good, scriptable tools, as these can drastically increase the
efficiency of your work. What took hours in the HEK (e.g. refactoring tags or
regenerating bitmaps as lossless) takes minutes in Invader. As such, command
line has to be first class!

For example, most shells support tab completion. If Invader is correctly set up
in your path and you type "inv" and press TAB, it'll autocomplete to "invader-".
You can then type "de" after that and press TAB again to autocomplete to
"invader-dependency" (or "invader-dependency.exe" on Windows). This means you've
typed 18-22 characters in just 7 keystrokes! That's 2x-3x as efficient. But if
it was just one executable, you would only be able to tab complete the word
"invader".

Also, if Invader one executable, arguments would need rethought. Consider these
two commands:

```
invader-build levels/test/bloodgulch/bloodgulch -g gbx-custom -r always
invader-dependency levels/test/bloodgulch/bloodgulch -r
```

How would this be done as a single executable? Well, tool.exe does the
verb-then-argument method:

```
invader build levels/test/bloodgulch/bloodgulch -g gbx-custom -r always -t tags
invader dependency levels/test/bloodgulch/bloodgulch -r -t tags
```

But now there's an issue! The -r argument in invader-build takes one argument,
but it doesn't in invader-dependency! There's no way to know how many arguments
-r takes or even what -r does UNTIL it has read the first argument.

Invader is written to process hyphenated parameters, first, and then parse any
remaining arguments second, but you can't just change it to process the
non-hyphenated arguments since "tags", "always", and "gbx-custom" do not start
with a hyphen but are part of a hyphenated parameter (which, again, it doesn't
know this yet!). So, it'd have to be written to specifically read the very first
argument before doing anything. This is certainly doable, but it's much harder
to document, and having argument order not usually matter is typically more
elegant to write in - both for developing the tools and actually using them.

We also want to avoid tool.exe-isms as much as possible. The tool.exe way is to
pass all of the arguments in a set order. Sure, it's faster to write tool.exe
since you don't have to do any sort of complicated argument parsing, but if you
get it in the wrong order, it may not work as expected.

For example, these are interpreted differently:

```
tool lightmaps levels\test\bloodgulch\bloodgulch bloodgulch 1 0.1
tool lightmaps levels\test\bloodgulch\bloodgulch bloodgulch 0.1 1
```

Something like "-f -q 0.00001" or "-q 0.00001 -f" is fine. Invader having
arguments you can usually specify in any order prevents errors! This makes
Invader far more user friendly, especially for beginners getting into
command-line and/or CE modding. The tool.exe way is not, and the lack of
well-made documentation further compounds these issues.

Also, a Vulkan renderer is being developed for Invader's scenario editor, but
not everyone has Vulkan! You may have an outdated GPU (e.g. an Intel HD 2000) or
no GPU (i.e. a "headless" system - very common in servers). If a program linked
against Vulkan (linking is generally the best way to use a library), then unless
you had Vulkan installed, the program wouldn't run.

For Invader, if you do NOT have Vulkan, then you simply wouldn't be able to run
the scenario editor. However, if the entire toolkit is a single executable, you
couldn't run Invader at all! If you just want to do simple tasks such as tag
extraction or tag stripping, you aren't going to need a renderer. Sure, there
are ways around this. Invader could be provided in such a way where you had a
Vulkan-less build, but this would confuse users and create a maintenance burden.
Invader could load an external renderer library at runtime, but this would
defeat the point of making Invader into one executable.

### Are there any GUI tools?
Officially, only one of these tools has a graphical user interface,
invader-edit-qt. However, there is a program you can optionally download called
[six-shooter](https://github.com/SnowyMouse/six-shooter) which provides a
graphical interface for some of Invader's other tools.

### Are 32-bit Windows builds available?
Only 64-bit builds are uploaded to [Nightly Builds]. You can compile Invader
for 32-bit x86 Windows, but due to architectural and performance differences,
these builds are not recommended. People who are unsure may even download a
32-bit build even though a 64-build will work better. Nearly all desktop x86 PCs
made in the past decade come with a 64-bit operating system.

### Can invader-build create .yelo maps?
Officially, invader-build only creates maps for officially-released versions of
the game. The .yelo file format is specific to Open Sauce, a *mod* of Halo
Custom Edition. Therefore, the Invader project does not support it. However,
this does not mean that you can't make a fork of Invader that supports it, and
there are people who have said they were willing to do this.

### Can invader-build create Xbox maps?
Yes. Pass `-g xbox-ntsc` (for NTSC Halo) into invader-build. You may need to
fix some tags if using tags for other versions of the game.

### The HEK says my bitmap tag is "too large" when opening.
The original HEK has a 16 MiB limitation for bitmap data. You can install the
[newer MCC Halo CE: Mod Tools] and use this for your workflow as it contains a
number of fixes and improvements such as more accurate BSP generation, higher
limits, and fixed shaders.

It can be downloaded free of charge on Steam even if you do not own Halo: Combat
Evolved Anniversary on Steam.

[newer MCC Halo CE: Mod Tools]: https://store.steampowered.com/app/1532190/Halo_CE_Mod_Tools__MCC/

### How close to completion is Invader?
There is still a lot to do in Invader. Check the [Issues] page for more
information.

[Issues]: https://github.com/SnowyMouse/invader/issues

### Should I use invader-build for my map right now?
It depends.

Because tool.exe has a number of bugs and invader-build does quite a few things
tool.exe doesn't do (such as better error checking), there are a few reasons to
use invader-build over tool.exe for multiplayer maps.

However, invader-build isn't finished and has a number of issues that need
ironed out before this FAQ can confidently recommend it for *every* use case,
while tool.exe is basically the reference when it comes to building cache files.

However, we do ask that you consider testing invader-build so we can improve it
to a point where it can be a better and free replacement for tool.exe.

[Staying up-to-date]: #staying-up-to-date
[Contributing]: #contributing
[Getting Invader]: #getting-invader
[Programs]: #programs
[Frequently asked questions]: #frequently-asked-questions

[I get errors when building HEK tags or tags extracted with invader-extract]: #i-get-errors-when-building-hek-tags-or-tags-extracted-with-invader-extract
[What operating systems are supported?]: #what-operating-systems-are-supported
[Who owns my map when I build it?]: #who-owns-my-map-when-i-build-it
[Why GPL and not MIT or BSD?]: #why-gpl-and-not-mit-or-bsd
[Why is Invader multiple programs?]: #why-is-invader-multiple-programs
[Are there any GUI tools?]: #are-there-any-gui-tools
[Are 32-bit Windows builds available?]: #are-32-bit-windows-builds-available
[Can invader-build create .yelo maps?]: #can-invader-build-create-yelo-maps
[Can invader-build create Xbox maps?]: #can-invader-build-create-xbox-maps
[The HEK says my bitmap tag is "too large" when opening.]: #the-hek-says-my-bitmap-tag-is-too-large-when-opening
[How close to completion is Invader?]: #how-close-to-completion-is-invader
[Should I use invader-build for my map right now?]: #should-i-use-invader-build-for-my-map-right-now

[invader-archive]: #invader-archive
[invader-bitmap]: #invader-bitmap
[invader-bludgeon]: #invader-bludgeon
[invader-build]: #invader-build
[invader-collection]: #invader-collection
[invader-compare]: #invader-compare
[invader-convert]: #invader-convert
[invader-dependency]: #invader-dependency
[invader-edit]: #invader-edit
[invader-edit-qt]: #invader-edit-qt
[invader-extract]: #invader-extract
[invader-font]: #invader-font
[invader-index]: #invader-index
[invader-info]: #invader-info
[invader-model]: #invader-model
[invader-recover]: #invader-recover
[invader-refactor]: #invader-refactor
[invader-resource]: #invader-resource
[invader-script]: #invader-script
[invader-sound]: #invader-sound
[invader-string]: #invader-string
[invader-strip]: #invader-strip
