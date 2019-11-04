# Invader
Invader is an open source toolkit for creating maps and assets for Halo: Combat
Evolved on the PC. See LICENSE for more information about the license.

## Getting started
This readme addresses a few topics:
- [Contributing]
- [Getting Invader]
- [Programs]
- [Frequently asked questions]

## Contributing
See CONTRIBUTING.md.

## Getting Invader
Invader can be obtained by either downloading pre-compiled binaries or
compiling from source.

You can also download precompiled [Nightly Builds].

### Building Invader
If you got this readme from an archive containing pre-compiled Invader
binaries, this section probably doesn't apply to you, but you are welcome to
compile Invader. Regardless, you can browse and download the source code for
free on [GitHub].

If you use Arch Linux, the [Arch Linux AUR] has a package you can use to build
Invader.

#### Dependencies
Invader depends on software in order for it to build and work properly. This
section lists the dependencies required to fully utilize Invader. Note that
some of these dependencies may have their own dependencies.

##### Required dependencies
- C++17 compiler
- C99 compiler
- CMake 3.10 or newer
- Python 3.7 or newer

##### Optional dependencies
- LibArchive (invader-archive)
- LibTIFF (invader-bitmap)
- zlib (invader-bitmap)
- freetype (invader-font)
- git (git commit hash in version - build only)

#### Compiling (Windows)
You can compile Invader using MSYS2 and MinGW. Make sure that you install all
of the required dependencies through MSYS2 and not their respective Windows
installers. See README.md for a list of the requirements. You can use the
`pacman` command to install these dependencies. Also, make sure that when you
compile Invader, you are using the 64-bit MSYS2 shell (or 32-bit if you need
a 32-bit build).

The rest of the instructions are the same as POSIX, except that when you go to
use the `make` command, you specify that you want CMake to generate MSYS
Makefiles. You can do so using `-G "MSYS Makefiles"` like this:

```
cmake ../invader -DCMAKE_BUILD_TYPE=Release -G "MSYS Makefiles"
```

If you forget to do this, then CMake may create a Microsoft Visual Studio
solution, instead. Invader will probably work if built for MSVC, but the
dependencies will have to be obtained differently, and the command to compile
is different. If you accidentally do this, delete all of the files in
`invader-build` and re-run the `cmake` command.

#### Compiling (POSIX)
First, you will need to download the Invader repository onto your computer. You
can do this using the command:

```
git clone https://github.com/Kavawuvi/invader
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
with Halo Custom Edition's tool.exe, as well as make things easier for me to
work with, this project is split into different programs.
- [invader-archive]
- [invader-bitmap]
- [invader-build]
- [invader-dependency]
- [invader-font]
- [invader-indexer]
- [invader-info]
- [invader-resource]

### invader-archive
This program generates a .tar.xz archive containing all of the tags used to
build a map.

```
Usage: invader-archive [options] <scenario | -s tag.class>

Generate .tar.xz archives of the tags required to build a cache file.

Options:
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -o --output <file>           Output to a specific file. Extension must be
                               .tar.xz.
  -P --fs-path                 Use a filesystem path for the tag.
  -s --single-tag              Archive a tag tree instead of a cache file.
  -t --tags <dir>              Use the specified tags directory. Use multiple
                               times to add more directories, ordered by
                               precedence.
```

### invader-bitmap
This program generates bitmap tags from images. For source images, .tif, .tiff,
.png, .tga, and .bmp extensions are
supported.

```
Usage: invader-bitmap [options] <bitmap-tag>

Create or modify a bitmap tag.

Options:
  -B --budget <length>         Set max length of sprite sheet. Can be 32, 64,
                               128, 256, or 512. Default (new tag): 32
  -C --budget-count <count>    Set maximum number of sprite sheets. Setting
                               this to 0 disables budgeting. Default (new tag):
                               0
  -d --data <path> <path>      Set the data directory.
  -D --dithering <channels>    Apply dithering to 16-bit, dxtn, or p8 bitmaps.
                               Specify channels with letters (i.e. argb).
  -f --detail-fade <factor>    Set detail fade factor. Default (new tag): 0.0
  -F --format                  Pixel format. Can be: 32-bit, 16-bit,
                               monochrome, dxt5, dxt3, or dxt1. Default (new
                               tag): 32-bit<type>
  -h --help                    Show this list of options.
  -H --bump-height <height>    Set the apparent bumpmap height from 0 to 1.
                               Default (new tag): 0.02
  -i --info                    Show license and credits.
  -I --ignore-tag              Ignore the tag data if the tag exists.
  -m --mipmap-count <count>    Set maximum mipmaps. Default (new tag): 32767
  -p --bump-palettize <val>    Set the bumpmap palettization setting. This will
                               not work with stock Halo. Can be: off or on.
                               Default (new tag): off
  -P --fs-path                 Use a filesystem path for the data.
  -s --mipmap-scale <type>     Mipmap scale type. Can be: linear,
                               nearest-alpha, nearest. Default (new tag):
                               linear
  -t --tags <path>             Set the data directory.
  -T --type <type>             Set the type of bitmap. Can be: 2d, 3d, cubemap,
                               interface, or sprite. Default (new tag): 2d
```

#### Uncompressed bitmap formats
These formats are uncompressed and use explicit (not interpolated) RGB and/or
alpha values. This results in higher quality bitmaps than using any of the
block compressed formats, but it comes with a file size tradeoff. Note that
32-bit bitmaps are slightly buggy on stock Halo PC without a mod (e.g.
[Chimera]).

If you use 16-bit, then using dithering (`-D rgb` or `-D argb` if you want
dithered alpha) may help with banding.

The exact storage format used will depend on the bitmap:
- If 32-bit is specified and all pixels have 100% alpha, then X8R8G8B8 is used.
  Otherwise, A8R8G8B8 is used.
- If 16-bit is specified and all pixels have 100% alpha, then R5G6B5 is used.
  If pixels have either 0% or 100% alpha, A1R5G5B5 is used. Otherwise, A4R4G4B4
  is used.

Format   | Storage  | Bits/px | Alpha | Red   | Green | Blue  | Notes
-------- | -------- | ------- | ----- | ----- | ----- | ----- | ----------
32-bit   | A8R8G8B8 | 32      | 8-bit | 8-bit | 8-bit | 8-bit |
32-bit   | X8R8G8B8 | 32      |       | 8-bit | 8-bit | 8-bit | 100% alpha
16-bit   | R5G6B5   | 16      |       | 5-bit | 6-bit | 5-bit | 100% alpha
16-bit   | A1R5G5B5 | 16      | 1-bit | 5-bit | 5-bit | 5-bit |
16-bit   | A4R4G4B4 | 16      | 4-bit | 4-bit | 4-bit | 4-bit |

#### Block-compressed bitmap formats
These formats utilize block compression. Basically, each bitmap is separated
into 4x4 blocks, and each block has two 16-bit (R5G6B5) colors which are
interpolated at runtime. This provides massive space savings but at a
significant loss in quality. Since the smallest block is 4x4, then mipmaps
smaller than 4x4 will not be generated, nor will you be able to make bitmaps
smaller than 4x4 while still being block-compressed.

There is no difference in RGB quality between dxt1, dxt3, or dxt5. The only
difference is in how alpha is handled. Therefore, if dxt3 or dxt5 is specified
and a bitmap does not have any alpha, then dxt1 will automatically be used to
keep the size as small as possible.

Format | Bits/px | Alpha              | Notes
------ | ------- | ------------------ | --------------------------------------
DXT1   | 4       |                    | 100% alpha
DXT3   | 8       | 4-bit explicit     | Better for shapes like HUDs
DXT5   | 8       | 4-bit interpolated | Better for alpha gradients like clouds

#### More formats
These formats were originally available on Xbox and do not work on stock Halo
PC without a mod (e.g. [Chimera]). They all use explicit RGB and/or alpha.

If you use monochrome with a monochrome bitmap used as input, then there will
be no loss in quality.

Format       | Storage  | Bits/px | Alpha   | RGB     | Notes
------------ | -------- | ------- | ------- | ------- | ---------------------
monochrome   | A8Y8     | 16      | 8-bit   | 8-bit   | Intensity (R=G=B)
monochrome   | A8       | 8       | 8-bit   |         | 100% intensity
monochrome   | Y8       | 8       |         | 8-bit   | 100% alpha
palettized   | P8       | 8       | Indexed | Indexed | Bump compression only

### invader-build
This program builds cache files.

```
Usage: invader-build [options] <scenario>

Build cache files for Halo Combat Evolved on the PC.

Options:
  -a --always-index-tags       Always index tags when possible. This can speed
                               up build time, but stock tags can't be modified.
  -c --forge-crc <crc>         Forge the CRC32 value of the map after building
                               it.
  -g --game-engine <id>        Specify the game engine. Valid engines are:
                               custom (default), retail, demo, dark
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -m --maps <dir>              Use a specific maps directory.
  -n --no-external-tags        Do not use external tags. This can speed up
                               build time at a cost of a much larger file size.
  -o --output <file>           Output to a specific file.
  -P --fs-path                 Use a filesystem path for the tag.
  -q --quiet                   Only output error messages.
  -t --tags <dir>              Use the specified tags directory. Use multiple
                               times to add more directories, ordered by
                               precedence.
  -w --with-index <file>       Use an index file for the tags, ensuring the
                               map's tags are ordered in the same way.
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
  -l --level <level>           Set the compression level. Must be between 1 and
                               22. Values > 19 use more memory. Default: 3
  -o --output <file>           Emit the resulting map at the given path. By
                               default, this is the map path (overwrite).
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

### invader-font
This program generates font tags.

```
Usage: invader-font [options] <font-tag>

Create font tags from TTF files.

Options:
  -d --data <dir>              Set the data directory.
  -h --help                    Show this list of options.
  -i --font-size <px>          Set the font size in pixels.
  -P --fs-path                 Use a filesystem path for the tag.
  -t --tags <dir>              Set the tags directory.
```

### invader-indexer
This program builds index files for usage with `--with-index` with invader-build.

```
Usage: invader-indexer <input map> <output index>
```

### invader-info
This program displays metadata of a cache file.

```
Usage: ./invader-info [option] <map>

Display map metadata.

Options:
  -h --help                    Show this list of options.
  -t --type                    Type of data to show. Can be overview (default),
                               compressed, crc32, dirty, engine, map-type,
                               protected, scenario, scenario-path, tag-count
```

### invader-resource
This program builds resource maps. Only maps with stock tags can be built.
These files are not guaranteed to work with existing cache files.

```
Usage: invader-resource <options>

Create resource maps.

Options:
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -m --maps <dir>              Set the maps directory.
  -R --retail                  Build a retail resource map (bitmaps/sounds
                               only)
  -t --tags <dir>              Use the specified tags directory. Use multiple
                               times to add more directories, ordered by
                               precedence.
  -T --type <type>             Set the resource map. This option is required
                               for creating maps. Can be: bitmaps, sounds, or
                               loc.
```

## Frequently asked questions
These are a selection of questions that have been asked over the course of
Invader's development.

- [What operating systems are supported?]
- [Are 32-bit Windows Nightly Builds available?]
- [Can invader-build create .yelo maps?]
- [Can invader-build create Xbox maps?]
- [The HEK says my bitmap tag is "too large" when opening.]
- [How close to completion is Invader?]
- [Should I use invader-bitmap or tool.exe?]
- [Should I use invader-build or tool.exe?]

### What operating systems are supported?
Invader does not support any one operating system, but Invader is developed on
Arch Linux, and 64-bit Windows builds are available via [Nightly Builds].

### Are 32-bit Windows Nightly Builds available?
Only 64-bit builds are uploaded to [Nightly Builds]. You can compile Invader
for 32-bit x86 Windows.

The reason 32-bit builds are not provided is because 32-bit builds are slower
and more limited than x86_64 due to architectural differences. People who are
unsure may also download a 32-bit build even though a 64-build will work
better. Nearly all desktop PCs made today come with a 64-bit operating system.

### Can invader-build create .yelo maps?
Officially, invader-build only creates maps for the Gearbox port of Halo on the
PC. The .yelo file format is specific to Open Sauce, a mod of Halo Custom
Edition. Therefore, the Invader project does not support it. However, this does
not mean that you can't make a fork of Invader that supports it, and there are
people who have said they were willing to do this.

### Can invader-build create Xbox maps?
Officially, invader-build only creates maps for the Gearbox port of Halo on the
PC. While Xbox maps are very similar in format to PC maps, there exists enough
differences to make supporting the Xbox version non-trivial. Kavawuvi also does
not have a modded Xbox or a retail copy of the Xbox version of the game, so
there is no means to debug or test.

### The HEK says my bitmap tag is "too large" when opening.
The HEK has a 16 MiB limitation for bitmap tags. Invader does not have this
limitation, and you can use the MEK to view bitmap tags that exceed 16 MiB.
Halo PC also does not have any problems loading bitmaps that exceed 16 MiB.
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

### Should I use invader-build or tool.exe?
It depends. Right now, invader-build isn't finished and has a number of issues
that need ironed out before this FAQ can confidently recommend it for *every*
use case, while tool.exe is basically the reference when it comes to building
cache files.

Most singleplayer maps will probably work, but until the stock campaign works
as it did when built with tool.exe, then it is not recommended to use
invader-build for singleplayer maps except for testing.

Most multiplayer maps work fine when built under invader-build. Because
tool.exe has a number of bugs and invader-build does quite a few things
tool.exe doesn't do, there are a few reasons to use invader-build over tool.exe
for multiplayer maps.

[GitHub]: https://github.com/Kavawuvi/invader
[Arch Linux AUR]: https://aur.archlinux.org/packages/invader-git/
[Nightly Builds]: https://invader.opencarnage.net/builds/nightly/download-latest.html
[Chimera]: https://chimera.opencarnage.net

[Contributing]: #contributing
[Getting Invader]: #getting-invader
[Programs]: #programs
[Frequently asked questions]: #frequently-asked-questions

[What operating systems are supported?]: #what-operating-systems-are-supported
[Are 32-bit Windows Nightly Builds available?]: #are-32-bit-windows-nightly-builds-available
[Can invader-build create .yelo maps?]: #can-invader-build-create-yelo-maps
[Can invader-build create Xbox maps?]: #can-invader-build-create-xbox-maps
[The HEK says my bitmap tag is "too large" when opening.]: #the-hek-says-my-bitmap-tag-is-too-large-when-opening
[How close to completion is Invader?]: #how-close-to-completion-is-invader
[Should I use invader-bitmap or tool.exe?]: #should-i-use-invader-bitmap-or-toolexe
[Should I use invader-build or tool.exe?]: #should-i-use-invader-build-or-toolexe

[invader-archive]: #invader-archive
[invader-bitmap]: #invader-bitmap
[invader-build]: #invader-build
[invader-dependency]: #invader-dependency
[invader-font]: #invader-font
[invader-indexer]: #invader-indexer
[invader-info]: #invader-info
[invader-resource]: #invader-resource
