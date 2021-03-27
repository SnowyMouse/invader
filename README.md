# Invader
Invader is an open source toolkit for creating maps and assets for Halo: Combat
Evolved.

Our Discord server is https://discord.gg/RCX3nvw

The official source code repository is https://github.com/SnowyMouse/invader

## License
Invader is licensed under version 3.0 of the GNU General Public License. Note
that Invader is NOT licensed under any later or previous version of the GNU
GPL.

See COPYING for a copy of version 3 of the GNU General Public License.

The ADPCM encoder and MosesofEgypt's Xbox ADPCM modifications are BSD and MIT,
respectively. Licenses can be found at `src/sound/adpcm_xq/license.txt` and
`src/sound/reclaimer-license.txt`.

Roboto Mono is from Google Fonts under [Apache License, Version 2.0].

[Apache License, Version 2.0]: http://www.apache.org/licenses/LICENSE-2.0

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
(`0.26.1`), commit number (`r1776`), and commit hash (`9db4cc5`).

Next, go to the [commits] and check the commit hash against the topmost commit.
If it is the same, then you are using the latest version of Invader.

Invader is not finished, so it is important to stay up-to-date to obtain the
latest features and fixes. Staying on an old version means staying on unfinished
software that likely has known issues.

[commits]: https://github.com/SnowyMouse/invader/commits/master

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
- C++17 compiler with support for `filesystem`
- C99 compiler
- CMake 3.12 or newer
- Python 3.7 or newer
- Zstandard 1.3 or newer
- LibTIFF 3.6 or newer
- libvorbis 1.3.6 or newer
- libsamplerate 0.1.9 or newer
- Qt5
- zlib

##### Optional dependencies
- LibArchive ([invader-archive])
- freetype ([invader-font])
- git (git commit hash in version - build only)

#### Compiling (POSIX)
First, you will need to download the Invader repository onto your computer. You
can do this using the command:

```
git clone https://github.com/SnowyMouse/invader
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
- [invader-compare]
- [invader-compress]
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
- [invader-sound]
- [invader-string]
- [invader-strip]

### invader-archive
This program generates a .tar.xz archive containing all of the tags used to
build a map.

```
Usage: invader-archive [options] <scenario | -s tag.class>

Generate .tar.xz archives of the tags required to build a cache file.

Options:
  -C --copy                    Copy instead of making an archive.
  -e --exclude <dir>           Exclude copying any tags that share a path with
                               a tag in specified directory. Use multiple times
                               to exclude multiple directories.
  -E --exclude-matched <dir>   Exclude copying any tags that are also located
                               in the specified directory and are functionally
                               the same. Use multiple times to exclude multiple
                               directories.
  -g --game-engine <id>        Specify the game engine. This option is required
                               if -s is not specified. Valid engines are:
                               custom, demo, retail, xbox, native
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -o --output <file>           Output to a specific file. Extension must be
                               .tar.xz unless using --copy which then it's a
                               directory.
  -P --fs-path                 Use a filesystem path for the tag.
  -s --single-tag              Archive a tag tree instead of a cache file.
  -t --tags <dir>              Use the specified tags directory. Use multiple
                               times to add more directories, ordered by
                               precedence.
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
  -B --budget <length>         Set max length of sprite sheet. Can be 32, 64,
                               128, 256, or 512. Default (new tag): 32
  -C --budget-count <count>    Set maximum number of sprite sheets. Setting
                               this to 0 disables budgeting. Default (new tag):
                               0
  -d --data <dir>              Use the specified data directory.
  -D --dithering <channels>    Apply dithering to 16-bit or p8 bitmaps. This
                               does not save in .bitmap tags. Can be: a, rgb,
                               or argb. Default: none
  -f --detail-fade <factor>    Set detail fade factor. Default (new tag): 0.0
  -F --format <type>           Pixel format. Can be: 32-bit, 16-bit,
                               monochrome, dxt5, dxt3, or dxt1. Default (new
                               tag): 32-bit
  -h --help                    Show this list of options.
  -H --bump-height <height>    Set the apparent bumpmap height from 0 to 1.
                               Default (new tag): 0.026
  -i --info                    Show license and credits.
  -I --ignore-tag              Ignore the tag data if the tag exists.
  -M --mipmap-count <count>    Set maximum mipmaps. Default (new tag): 32767
  -p --bump-palettize <val>    Set the bumpmap palettization setting. Can be:
                               off or on. Default (new tag): off
  -P --fs-path                 Use a filesystem path for the data.
  -R --regenerate              Use the bitmap tag's compressed color plate data
                               as data.
  -s --mipmap-scale <type>     Mipmap scale type. This does not save in .bitmap
                               tags. Can be: linear, nearest_alpha, nearest.
                               Default (new tag): linear
  -t --tags <dir>              Use the specified tags directory.
  -T --type <type>             Set the type of bitmap. Can be: 2d_textures,
                               3d_textures, cube_maps, interface_bitmaps, or
                               sprites. Default (new tag): 2d
  -u --usage <usage>           Set the bitmap usage. Can be: alpha_blend,
                               default, height_map, detail_map, light_map,
                               vector_map. Default: default
```

Refer to [Creating a bitmap] for information on how to create bitmap tags.

[Creating a bitmap]: https://github.com/SnowyMouse/invader/wiki/Creating-a-bitmap

### invader-bludgeon
This program convinces tags to work with Invader.

```
Usage: invader-bludgeon [options] <-a | tag.class>

Convinces tags to work with Invader.

Options:
  -a --all                     Bludgeon all tags in the tags directory.
  -h --help                    Show this list of options.
  -i --info                    Show license and credits.
  -j --threads                 Set the number of threads to use for parallel
                               bludgeoning when using --all. Default: CPU
                               thread count
  -P --fs-path                 Use a filesystem path for the tag path if
                               specifying a tag.
  -t --tags <dir>              Use the specified tags directory.
  -T --type                    Type of bludgeoning. Can be: invalid-enums,
                               out-of-range, invalid-strings,
                               invalid-reference-classes,
                               invalid-model-markers, missing-script-source,
                               incorrect-sound-buffer, missing-vertices,
                               nonnormal-vectors, invalid-uppercase-references,
                               excessive-script-nodes, none, everything
                               (default: none)
```

### invader-build
This program builds cache files.

```
Usage: invader-build [options] -g <target> <scenario>

Build a cache file for a version of Halo: Combat Evolved.

Options:
  -a --always-index-tags       Always index tags when possible. This can speed
                               up build time, but stock tags can't be modified.
  -b --stock-resource-bounds   Only index tags if the tag's index is within
                               stock Custom Edition's resource map bounds.
                               (Custom Edition only)
  -B --build-version           Set the build version. This is used on the Xbox
                               version of the game (by default it's
                               01.10.12.2276 on Xbox and the Invader version on
                               other engines)
  -c --compress                Compress the cache file.
  -C --forge-crc <crc>         Forge the CRC32 value of the map after building
                               it.
  -E --extend-file-limits      Extend file size limits beyond what is allowed
                               by the target engine to its theoretical maximum
                               size. This may create a map that will not work
                               without a mod.
  -g --game-engine <id>        Specify the game engine. This option is
                               required. Valid engines are: custom, demo,
                               native, retail, xbox, xbox-tw, xbox-jp,
                               mcc-custom, mcc-retail
  -h --help                    Show this list of options.
  -H --hide-pedantic-warnings  Don't show minor warnings.
  -i --info                    Show credits, source info, and other info.
  -l --level <level>           Set the compression level. Must be between 0 and
                               19. If compressing an Xbox or MCC map, this will
                               be clamped from 0 to 9. Default: 19
  -m --maps <dir>              Use the specified maps directory.
  -n --no-external-tags        Do not use external tags. This can speed up
                               build time at a cost of a much larger file size.
  -N --rename-scenario <name>  Rename the scenario.
  -o --output <file>           Output to a specific file.
  -O --optimize                Optimize tag space. This will drastically
                               increase the amount of time required to build
                               the cache file.
  -P --fs-path                 Use a filesystem path for the tag.
  -q --quiet                   Only output error messages.
  -t --tags <dir>              Use the specified tags directory. Use multiple
                               times to add more directories, ordered by
                               precedence.
  -u --uncompressed            Do not compress the cache file. This is default
                               for demo, retail, and custom engines.
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

### invader-compare
This program compares tags against maps, maps against maps, and tags against
tags.

```
Usage: invader-compare [options] <-I <opts>> <-I <opts>> [<-I <opts>> ...]

Compare tags against other tags.

Options:
  -a --all                     Only match if tags are in all inputs
  -B --by-path <path-type>     Set what tags get compared against other tags.
                               By default, only tags with the same relative
                               path are checked. Using "any" ignores paths
                               completely (useful for finding duplicates when
                               both inputs are different) while "different"
                               only checks tags with different paths (useful
                               for finding duplicates when both inputs are the
                               same). Can be: any, different, or same (default)
  -c --class                   Add a tag class to check. If no tag classes are
                               specified, all tag classes will be checked.
  -f --functional              Precompile the tags before comparison to check
                               for only functional differences.
  -G --ignore-resources        Ignore resource maps for the current map input.
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -I --input                   Add an input directory
  -m --maps                    Add a maps directory to the input to specify
                               where to find resource files for a map.
  -M --map                     Add a map to the input. Only one map can be
                               specified per input. If a maps directory isn't
                               specified, then the map's directory will be
                               used.
  -p --precision               Allow for slight differences in floats to
                               account for precision loss.
  -s --show                    Can be: all, matched, or mismatched. Default:
                               all
  -t --tags                    Add a tags directory to the input. Specify
                               multiple tag directories in order of precedence
                               for the input.
  -v --verbose                 Output more information on the differences
                               between tags to standard output. This will not
                               work with -f
```

### invader-compress
This program compresses cache files.

```
Usage: invader-compress [options] <map>

Compress cache files.

Options:
  -d --decompress              Decompress instead of compress.
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -l --level <level>           Set the compression level. Must be between 0 and
                               19. If compressing an MCC map, this will be
                               clamped from 0 to 9. Default: 19
  -M --mcc                     Use MCC-style (de)compression
  -o --output <file>           Emit the resulting map at the given path. By
                               default, this is the map path (overwrite).
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
  -P --fs-path                 Use a filesystem path for the tag path if
                               specifying a tag.
  -s --single-tag              Convert a specific tag. This can be specified
                               multiple times for multiple tags, but cannot be
                               used with -a.
  -t --tags <dir>              Set the input tags directory.
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
  -t --tags <dir>              Use the specified tags directory. Use multiple
                               times to add more directories, ordered by
                               precedence.
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
  -i --info                    Show license and credits.
  -I --insert <key> <#> <pos>  Add # structs to the given index or "end" if the
                               end of the array.
  -l --list                    List all elements in a tag.
  -L --list-values             List all elements and values in a tag. This may
                               be slow on large tags.
  -M --move <key> <pos>        Swap the selected structs with the structs at
                               the given index or "end" if the end of the
                               array. The regions must not intersect.
  -N --new                     Create a new tag
  -o --output <file>           Output the tag to a different path rather than
                               overwriting it.
  -O --save-as <tag>           Output the tag to a different path relative to
                               the tags directory rather than overwriting it.
  -P --fs-path                 Use a filesystem path for the font data or tag
                               file.
  -S --set <key> <val>         Set the value at the given key to the given
                               value.
  -t --tags <dir>              Use the specified tags directory.
```

### invader-edit-qt
This program edits tags in a Qt-based GUI.

```
Usage: invader-edit-qt [options] [<tag1> [tag2] [...]]

Edit tag files.

Options:
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -n --no-safeguards           Allow all tag data to be edited (proceed at your
                               own risk)
  -P --fs-path                 Use a filesystem path for the tag path if
                               specifying a tag.
  -t --tags <dir>              Use the specified tags directory. Use multiple
                               times to add more directories, ordered by
                               precedence.
```

### invader-extract
This program extracts tags from cache files.

```
Usage: invader-extract [options] <map>

Extract data from cache files.

Options:
  -G --ignore-resources        Ignore resource maps.
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info
  -m --maps <dir>              Use the specified maps directory.
  -n --non-mp-globals          Enable extraction of non-multiplayer .globals
  -O --overwrite               Overwrite tags if they already exist
  -r --recursive               Extract tag dependencies
  -s --search <expr>           Search for tags (* and ? are wildcards); use
                               multiple times for multiple queries
  -t --tags <dir>              Use the specified tags directory.
```

### invader-font
This program generates font tags.

```
Usage: invader-font [options] <font-tag>

Create font tags from OTF/TTF files.

Options:
  -d --data <dir>              Set the data directory.
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -P --fs-path                 Use a filesystem path for the font file.
  -s --font-size <px>          Set the font size in pixels.
  -t --tags <dir>              Set the tags directory.
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
                               (default), build, compressed, compression_ratio,
                               crc32, crc32_mismatched, dirty, engine,
                               external_bitmap_indices, external_bitmaps,
                               external_indices, external_loc_indices,
                               external_pointers, external_sound_indices,
                               external_sounds, external_tags, languages,
                               map_type, protection, scenario, scenario_path,
                               tag_count, stub_count, tags,
                               tags_external_bitmap_indices,
                               tags_external_loc_indices,
                               tags_external_pointers,
                               tags_external_sound_indices,
                               tags_external_indices, uncompressed_size
```

### invader-model
This program compiles model tags. It can compile both model and gbxmodel formats
directly from .JMS files.

JMS files should be placed in the data folder in a `models` folder in a folder
relative to where the model tag will be generated in the tags directory. So, if
you want to compile a model tags `weapons/pistol/pistol.model`, you would put
your JMS files in `data/weapons/pistol/pistol/models` or, if using legacy mode,
in `data/weapons/pistol/models`. A tutorial will be made sometime in the future
regarding this.

You can make JMS files for free using [Halo-Asset-Blender-Development-Toolset]
in [Blender].

[Halo-Asset-Blender-Development-Toolset]: https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
[Blender]: https://www.blender.org/

```
Usage: invader-model [options] <model-tag>

Compile a model tag.

Options:
  -d --data <dir>              Use the specified data directory.
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -L --legacy                  Use legacy behavior (use parent folder's
                               filename for the tag name for tool.exe backwards
                               compatibility).
  -P --fs-path                 Use a filesystem path for the tag path or data
                               directory.
  -t --tags <dir>              Use the specified tags directory. Additional
                               tags directories can be specified for searching
                               shaders, but the tag will be output to the first
                               one.
  -T --type <type>             Specify the type of model. Can be: model,
                               gbxmodel
```

### invader-recover
This program recovers source data from bitmaps (if color plate data is present),
models, string lists, tag collections, and scenario scripts

```
Usage: invader-recover [options] <tag.class>

Recover source data from tags.

Options:
  -d --tags <dir>              Use the specified data directory.
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -O --overwrite               Overwrite data if it already exists
  -P --fs-path                 Use a filesystem path for the tag path
                               directory.
  -t --tags <dir>              Use the specified tags directory.
```

### invader-refactor
This program renames and moves tag references.

```
Usage: invader-refactor <-M <mode>> [options]

Find and replace tag references.

Options:
  -c --class <f> <t>           Refactor all tags of a given class to another
                               class. All tags in the destination class must
                               exist. This can be specified multiple times but
                               cannot be used with --recursive or -M move.
  -D --dry-run                 Do not actually make any changes. This is useful
                               for checking for errors before committing
                               anything, although filesystem errors may not be
                               caught.
  -h --help                    Show this list of options.
  -i --info                    Show license and credits.
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
  -R --replace-string <a> <b>  Replaces all instances in a path of <a> with
                               <b>. This can be used multiple times for
                               multiple replacements. This cannot be used with
                               --class or --recursive.
  -r --recursive <f> <t>       Recursively move all tags in a directory. This
                               will fail if a tag is present in both the old
                               and new directories, it cannot be used with
                               no-move. This can only be specified once per
                               operation and cannot be used with --tag.
  -s --single-tag <path>       Make changes to a single tag, only, rather than
                               the whole tags directory.
  -t --tags <dir>              Use the specified tags directory. Use multiple
                               times to add more directories, ordered by
                               precedence.
  -T --tag <f> <t>             Refactor an individual tag. This can be
                               specified multiple times but cannot be used with
                               --recursive.
  -U --unsafe                  Do not require the destination tags to exist if
                               using no-move
```

### invader-resource
This program builds resource maps. Only maps with stock tags can be built.
These files are not guaranteed to work with existing cache files.

```
Usage: invader-resource [options] -T <type>

Create resource maps.

Options:
  -c --concatenate <file>      Concatenate against the resource map at a path.
  -g --game-engine <id>        Specify the game engine. This option is
                               required. Demo and retail maps also require
                               either -w or -M to be specified at least once.
                               Valid engines are: custom, demo, retail
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -m --maps <dir>              Set the maps directory.
  -M --with-map <file>         Use a map file for the tags. This can be
                               specified multiple times.
  -n --no-prefix               Don't use the "custom_" prefix when building a
                               Custom Edition resource map.
  -t --tags <dir>              Use the specified tags directory. Use multiple
                               times to add more directories, ordered by
                               precedence.
  -T --type <type>             Set the resource map. This option is required.
                               Can be: bitmaps, sounds, or loc.
  -w --with-index <file>       Use an index file for the tags, ensuring tags
                               are ordered in the same way (barring
                               duplicates).
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
  -d --data <dir>              Use the specified data directory.
  -F --format <fmt>            Set the format. Can be: 16-bit_pcm, ogg_vorbis,
                               or xbox_adpcm.
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -j --threads                 Set the number of threads to use for parallel
                               resampling and encoding. Default: CPU thread
                               count
  -l --compress-level <lvl>    Set the compression level. This can be between
                               0.0 and 1.0. For Ogg Vorbis, higher levels
                               result in better quality but worse sizes. For
                               FLAC, higher levels result in better sizes but
                               longer compression time, clamping from 0.0 to
                               0.8 (FLAC 0 to FLAC 8). Default: 1.0
  -P --fs-path                 Use a filesystem path for the data or tag.
  -r --sample-rate <Hz>        Set the sample rate in Hz. Halo supports 22050
                               and 44100. By default, this is determined based
                               on the input audio.
  -s --split                   Split permutations into 227.5 KiB chunks. This
                               is necessary for longer sounds (e.g. music) when
                               being played in the original Halo engine.
  -S --no-split                Do not split permutations.
  -t --tags <dir>              Use the specified tags directory. Use multiple
                               times to add more directories, ordered by
                               precedence.
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
  -d --data <dir>              Use the specified data directory.
  -f --format                  Set string list format. Can be unicode or
                               latin-1. Must be specified if a string tag is not
                               present.
  -h --help                    Show this list of options.
  -i --info                    Show license and credits.
  -P --fs-path                 Use a filesystem path for the text file.
  -t --tags <dir>              Use the specified tags directory.
```

### invader-strip
This program strips hidden data from tags and recalculates their CRC32 values.

```
Usage: invader-strip [options] <-a | tag.class>

Strips extra hidden data from tags.

Options:
  -a --all                     Strip all tags in the tags directory.
  -h --help                    Show this list of options.
  -i --info                    Show license and credits.
  -P --fs-path                 Use a filesystem path for the tag path if
                               specifying a tag.
  -t --tags <dir>              Use the specified tags directory.
```

## Frequently asked questions
These are a selection of questions that have been asked over the course of
Invader's development.
- [I get errors when building HEK tags or tags extracted with invader-extract]
- [What operating systems are supported?]
- [Who owns my map when I build it?]
- [Why GPL and not MIT or BSD?]
- [How can I get Invader under a different license?]
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
invader-bludgeon --all -T invalid-indices -T invalid-enums -T out-of-range
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
invader-strip --all
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
give you permission to use any of these assets. Consult a legal expert if you
need assurance on your rights.

### Why GPL and not MIT or BSD?
Invader uses version 3 of the GNU GPL because we feel that Invader and all
derivatives and modifications of Invader should stay free and open.

The Halo CE modding community has a habit of obfuscating knowledge that could
be used to help other people. This is often done through these means:
- [Map protection](http://forum.halomaps.org/index.cfm?page=topic&topicID=16885)
- [Writing closed source software](http://halo.isimaginary.com/)
- [Obfuscating software](http://forum.halomaps.org/index.cfm?page=topic&topicID=51196&start=112)
- [Keeping helpful information locked behind secret forums and groups](http://www.macgamingmods.com/forum/viewtopic.php?f=40&t=7948)

Many great accomplishments have been lost to time, as most of the people who
made them are no longer there to support what they have made.

Often times, people will say they will release the source code in the future,
or they will at least consider releasing the source code. In nearly all cases
of this, the source code was never released. The GNU GPL solves this problem
by requiring everyone to have access to the source code when binary code is
released.

As time goes on, unmaintained, closed-source software becomes more seemingly
broken, incomplete, or even incompatible. For example, the Halo Editing Kit has
numerous issues that prevent people from making the content they want to make,
yet [it is still the only way to fully create maps]. The worst part about this
is that, in many cases, people do not make any sort of effort to replace this
software with working software, as they do not feel it is worth the time or
effort. Therefore, the broken, unmaintained software continues being used. This
is where Invader comes in.

[it is still the only way to fully create maps]: https://opencarnage.net/index.php?/topic/7765-replacing-the-halo-editing-kit-with-open-source-software/

In our opinion, we feel that modding tools should be open for all to use,
share, change, and learn from. At the same time, we also feel that a
requirement for people give back to the community the source code to any
derivatives of our tools they have chosen to share is not too much to ask for,
as this ensures that modding stays free and open instead of left to stagnation
behind closed groups and obfuscation as modding this game has been in the past.

Also, a weak license like MIT has proven to not be helpful. A common argument is
that more people will be willing to use and contribute to it. However, a number
of open source mods, which were released as MIT and BSD, have been used in
closed source projects, or they have been converted to closed source with the
[original open source versions receiving zero contributions].

[original open source versions receiving zero contributions]: https://github.com/Chaosvex/HAC2

That is why Invader uses version 3 of the GNU GPL.

### How can I get Invader under a different license?
Officially, the only way to obtain Invader is through version 3.0 of the GNU
General Public License. We will not provide exception to anyone - not even
Bungie, Microsoft, or our own mothers.

In order to get Invader under a different license, you will need to write to
*both* [Snowy Mouse] and [Vaporeon], and you will need *unanimous* approval.

[Snowy Mouse]: https://github.com/SnowyMouse
[Vaporeon]: https://github.com/Vaporeon

### Are there any GUI tools?
Officially, only one of these tools has a graphical user interface,
invader-edit-qt. However, there is a program you can optionally download called
[six-shooter](https://github.com/SnowyMouse/six-shooter) which provides a
graphical interface for some of Invader's other tools.

### Are 32-bit Windows builds available?
Only 64-bit builds are uploaded to [Nightly Builds]. You can compile Invader
for 32-bit x86 Windows.

The reason 32-bit builds are not provided is because 32-bit builds are slower
and more limited than x86_64 due to architectural differences (e.g. fewer
registers to hold temporary data). People who are unsure may also download a
32-bit build even though a 64-build will work better. Nearly all desktop PCs
made today come with a 64-bit operating system.

### Can invader-build create .yelo maps?
Officially, invader-build only creates maps for officially-released versions of
the game. The .yelo file format is specific to Open Sauce, a *mod* of Halo
Custom Edition. Therefore, the Invader project does not support it. However,
this does not mean that you can't make a fork of Invader that supports it, and
there are people who have said they were willing to do this.

### Can invader-build create Xbox maps?
Yes. Pass `-g xbox` into invader-build. A guide for making Xbox maps is planned.

### The HEK says my bitmap tag is "too large" when opening.
The HEK has a 16 MiB limitation for bitmap tags. Invader does not have this
limitation, and you can use invader-edit-qt to view bitmap tags that exceed 16
MiB. Halo PC also does not have any problems loading bitmaps that exceed 16 MiB.
That said, some DirectX 9 GPUs and/or implementations won't support textures
larger than 2048x2048 (2D textures) or 256x256x256 (3D textures).

It is worth noting that invader-build and various other tools let you specify
multiple tag directories. If you need to use Sapien, you can put a lower
quality, compressed version of your bitmap in your main tags folder, and you
can put the higher quality version in a tags directory that takes priority.
Sapien will use the lower quality bitmap and load happily, and invader-build
will use the higher quality bitmap when building.

### How close to completion is Invader?
There is still a lot to do in Invader. Check the Issues page for more
information.

### Should I use invader-bitmap or tool.exe?
In this case, invader-bitmap is either mostly or completely feature-complete,
and it has a number of features tool.exe does not have, such as support for the
.tga format (goes well with Refinery's data extraction) as well as dithering.
Therefore, invader-bitmap is the superior choice.

### Should I use invader-build for my map right now?
It depends.

Most multiplayer maps will work without issue, particularly those without
scripts, but singleplayer maps may have issues. Because tool.exe has a number of
bugs and invader-build does quite a few things tool.exe doesn't do (such as
better error checking), there are a few reasons to use invader-build over
tool.exe for multiplayer maps.

However, invader-build isn't finished and has a number of issues that need
ironed out before this FAQ can confidently recommend it for *every* use case,
while tool.exe is basically the reference when it comes to building cache files.

However, we do ask that you consider testing invader-build so we can improve it
to a point where it can be a solid replacement to tool.exe.

[Staying up-to-date]: #staying-up-to-date
[Contributing]: #contributing
[Getting Invader]: #getting-invader
[Programs]: #programs
[Frequently asked questions]: #frequently-asked-questions

[I get errors when building HEK tags or tags extracted with invader-extract]: #i-get-errors-when-building-hek-tags-or-tags-extracted-with-invader-extract
[What operating systems are supported?]: #what-operating-systems-are-supported
[Who owns my map when I build it?]: #who-owns-my-map-when-i-build-it
[Why GPL and not MIT or BSD?]: #why-gpl-and-not-mit-or-bsd
[How can I get Invader under a different license?]: #how-can-i-get-invader-under-a-different-license
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
[invader-compare]: #invader-compare
[invader-compress]: #invader-compress
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
[invader-sound]: #invader-sound
[invader-string]: #invader-string
[invader-strip]: #invader-strip
