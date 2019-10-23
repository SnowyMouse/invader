# Invader
Invader is an open source toolkit for creating maps and assets for Halo: Combat Evolved on the PC. See LICENSE for more
information about the license.

## Getting started
This readme addresses a few topics:
* [Getting Invader](#getting-invader)
* [Programs](#programs)
* [Frequently asked questions](#frequently-asked-questions)

## Getting Invader
Invader can be obtained by either downloading pre-compiled binaries or compiling from source.

You can obtain Windows builds on [Open Carnage](https://invader.opencarnage.net/builds/nightly/download-latest.html).

### Building Invader
If you got this readme from an archive containing pre-compiled Invader binaries, this section probably doesn't apply to
you, but you are welcome to compile Invader. Regardless, you can browse and download the source code for free on
[GitHub](https://github.com/Kavawuvi/invader).

If you use Arch Linux, the [Arch Linux AUR](https://aur.archlinux.org/packages/invader-git/) has a package you can use
to build Invader.

#### Dependencies
Invader depends on software in order for it to build and work properly. This section lists the dependencies required to
fully utilize Invader. Note that some of these dependencies may have their own dependencies.

##### Required dependencies
* C++17 compiler
* C99 compiler
* CMake 3.10 or newer
* Python 3.7 or newer

##### Optional dependencies
* LibArchive (invader-archive)
* LibTIFF (invader-bitmap)
* zlib (invader-bitmap)
* freetype (invader-font)
* git (git commit hash in version - build only)

#### Compiling (Windows)
You can compile Invader using MSYS2 and MinGW. Make sure that you install all of the required dependencies through
MSYS2 and not their respective Windows installers. See README.md for a list of the requirements. You can use the
`pacman` command to install these dependencies. Also, make sure that when you compile Invader, you are using the 64-bit
MSYS2 shell.

The rest of the instructions are the same as POSIX, except that when you go to use the `make` command, you specify that
you want CMake to generate MSYS Makefiles. You can do so using `-G "MSYS Makefiles"` like this:

```
cmake ../invader -DCMAKE_BUILD_TYPE=Release -G "MSYS Makefiles"
```

If you forget to do this, then CMake may create a Microsoft Visual Studio solution, instead. Invader will probably
work if built for MSVC, but the dependencies will have to be obtained differently, and the command to compile is
different. If you accidentally do this, delete all of the files in `invader-build` and re-run the `cmake` command.

#### Compiling (POSIX)
First, you will need to download the Invader repository onto your computer. You can do this using the command:

```
git clone https://github.com/Kavawuvi/invader
```

Everything in this section, from this point on, assumes the Invader repository was cloned in a directory called
"invader" in the current directory.

Next, you will need to create an out-of-source build directory. You can use this command to make the build
directory and CD into it upon success:

```
mkdir invader-build && cd invader-build
```

Next, use the `cmake` command to set up your build directory, optionally specifying the build type to Release:

```
cmake ../invader -DCMAKE_BUILD_TYPE=Release
```

Lastly, you can compile this using the `make` command.

```
make
```

## Programs
To remove the reliance of one huge executable, something that has caused issues with Halo Custom Edition's tool.exe, as
well as make things easier for me to work with, this project is split into different programs.

### invader-archive
This program generates a .tar.xz archive containing all of the tags used to build a map.

**Usage:** `invader-archive [options] <scenario-tag | -s tag.class>`

| Option               | Description                                                 |
| -------------------- | ----------------------------------------------------------- |
| `--info,-i`          | Show credits, source info, and other info.                  |
| `--help,-h`          | Show help.                                                  |
| `--fs-path,-P`       | Use a filesystem path for the tag.                          |
| `--output,-o <file>` | Output to a specific file. Extension must be .tar.xz.       |
| `--single-tag,-s`    | Archive a tag tree instead of a cache file tree.            |
| `--tags,-t <dir>`    | Tags directory. Use multiple times to add tags directories. |

### invader-bitmap
This program generates bitmap tags from images. For source images, .tif, .tiff, .png, .tga, and .bmp extensions are
supported.

**Usage:** `invader-bitmap [options] <bitmap-data>`

| Option                      | Description                                                                                 |
| --------------------------- | ------------------------------------------------------------------------------------------- |
| `--info,-i`                 | Show credits, source info, and other info.                                                  |
| `--help,-h`                 | Show help.                                                                                  |
| `--fs-path,-P`              | Use a filesystem path for the bitmap data path.                                             |
| `--data,-d <dir>`           | Data directory.                                                                             |
| `--tags,-t <dir>`           | Tags directory.                                                                             |
| `--type,-T`                 | Set bitmap type. Can be `2d` (default), `3d`, `cubemap`, `interface`, or `sprite`.          |
| `--usage,-u <usage>`        | Set usage. Can be `default` (default) or `bumpmap`.                                         |
| `--dithering,-D <channels>` | Apply dithering for dxt, 16-bit, and p8. Specify channels with letters (i.e. `argb`).       |
| `--ignore-tag,-I`           | Ignore the tag data if the tag exists.                                                      |
| `--format,-F <format>`      | Pixel format. Can be `32-bit` (default), `16-bit`, `monochrome`, `dxt1`, `dxt3`, or `dxt5`. |
| `--mipmap-count,-m <count>` | Set maximum mipmap count. By default, this is 32767.                                        |
| `--mipmap-scale,-s <type>`  | Mipmap scaling. Can be `linear` (default), `nearest-alpha`, `nearest`, or `none`.           |
| `--detail-fade,-f <factor>` | Set fade-to-gray factor for mipmaps of detail maps. By default, this is 0.                  |
| `--spacing,-S <px>`         | Set minimum spacing between sprites in pixels. By default, this is 4.                       |
| `--bump-height,-H <height>` | Set apparent bump height from 0 to 1. By default, this is 0.02.                             |
| `--budget-count,-C <count>` | Set max number of sprite sheets. 0 (default) disables budgeting.                            |
| `--budget,-B <length>`      | Set max length of sprite sheet. By default, this is 32.                                     |

#### Uncompressed bitmap formats
These formats are uncompressed and use explicit (not interpolated) RGB and/or alpha values. This results in higher
quality bitmaps than using any of the block compressed formats, but it comes with a file size tradeoff. Note that
32-bit bitmaps are slightly buggy on stock Halo PC without a mod (e.g. [Chimera](https://github.com/Kavawuvi/Chimera)).

If you use 16-bit, then using dithering (`-D rgb` or `-D argb` if you want dithered alpha) may help with banding.

The exact storage format used will depend on the bitmap:
* If 32-bit is specified and all pixels have 100% alpha, then X8R8G8B8 is used. Otherwise, A8R8G8B8 is used.
* If 16-bit is specified and all pixels have 100% alpha, then R5G6B5 is used. If pixels have either 0% or 100% alpha,
A1R5G5B5 is used. Otherwise, A4R4G4B4 is used.

| Format   | Storage  | Bits/px | Alpha   | Red   | Green | Blue  | Notes                 |
| -------- | -------- | ------- | ------- | ----- | ----- | ----- | --------------------- |
| `32-bit` | A8R8G8B8 | 32      | 8-bit   | 8-bit | 8-bit | 8-bit |                       |
|          | X8R8G8B8 | 32      | (8-bit) | 8-bit | 8-bit | 8-bit | All pixels 100% alpha |
| `16-bit` | R5G6B5   | 16      |         | 5-bit | 6-bit | 5-bit | All pixels 100% alpha |
|          | A1R5G5B5 | 16      | 1-bit   | 5-bit | 5-bit | 5-bit |                       |
|          | A4R4G4B4 | 16      | 4-bit   | 4-bit | 4-bit | 4-bit |                       |

#### Block-compressed bitmap formats
These formats utilize block compression. Basically, each bitmap is separated into 4x4 blocks, and each block has two
16-bit (R5G6B5) colors which are interpolated at runtime. This provides massive space savings but at a significant
loss in quality. Since the smallest block is 4x4, then mipmaps smaller than 4x4 will not be generated, nor will you be
able to make bitmaps smaller than 4x4 while still being block-compressed.

There is no difference in RGB quality between dxt1, dxt3, or dxt5. The only difference is in how alpha is handled.
Therefore, if dxt3 or dxt5 is specified and a bitmap does not have any alpha, then dxt1 will automatically be used to
keep the size as small as possible.

| Format | Bits/px | Alpha              | RGB                 | Notes                                  |
| ------ | ------- | ------------------ | ------------------- | -------------------------------------- |
| `dxt1` | 4       |                    | 16-bit interpolated | All pixels 100% alpha                  |
| `dxt3` | 8       | 4-bit explicit     | 16-bit interpolated | Better for shapes like HUDs            |
| `dxt5` | 8       | 4-bit interpolated | 16-bit interpolated | Better for alpha gradients like clouds |

#### More formats
These formats were originally available on Xbox and do not work on stock Halo PC without a mod (e.g.
[Chimera](https://github.com/Kavawuvi/Chimera)). They all use explicit RGB and/or alpha.

If you use monochrome with a monochrome bitmap used as input, then there will be no loss in quality.

| Format       | Storage  | Bits/px | Alpha   | RGB     | Notes                     |
| ------------ | -------- | ------- | ------- | ------- | ------------------------- |
| `monochrome` | A8Y8     | 16      | 8-bit   | 8-bit   | Intensity (R=G=B)         |
|              | A8       | 8       | 8-bit   |         | All pixels 100% intensity |
|              | Y8       | 8       |         | 8-bit   | All pixels 100% alpha     |
| palettized   | P8       | 8       | Indexed | Indexed | Bumpmaps only             |

### invader-build
This program builds cache files.

**Usage:** `invader-build [options] <scenario-tag>`

| Option                    | Description                                                          |
| ------------------------- | -------------------------------------------------------------------- |
| `--info,-i`               | Show credits, source info, and other info.                           |
| `--help,-h`               | Show help.                                                           |
| `--fs-path,-P`            | Use a filesystem path for the scenario tag.                          |
| `--game-engine,-g` <type> | Set target map type. Can be `ce` (default), `retail`, `demo`, `dark` |
| `--maps,-m <dir>`         | Use a specific maps directory.                                       |
| `--tags,-t <dir>`         | Tags directory. Use multiple times to add tags directories.          |
| `--always-index-tags,-a`  | Always index tags with resource maps when possible.                  |
| `--no-external-tags,-n`   | Do not external tags from resource maps.                             |
| `--forge-crc,-c <crc>`    | Forge the CRC.                                                       |
| `--output,-o <file>`      | Output to a specific file.                                           |
| `--quiet,-q`              | Only output error messages.                                          |
| `--with-index,-w <file>`  | Use an index file for the tags.                                      |

### invader-crc
This program calculates the CRC32 of a map file. If the CRC value calculated differs from the value stored in the cache
file header, then a warning will be printed to standard error.

**Usage:** `invader-crc <map>`

### invader-dependency
This program finds tags that directly depend on a given tag.

**Usage:** `invader-dependency [options] <tag.class>`

| Option            | Description                                                 |
| ----------------- | ----------------------------------------------------------- |
| `--help,-h`       | Show help.                                                  |
| `--info,-i`       | Show credits, source info, and other info.                  |
| `--fs-path,-P`    | Use a filesystem path for the tag.                          |
| `--recursive,-R`  | Recursively get all depended tags.                          |
| `--reverse,-r`    | Find all tags that depend on the tag, instead.              |
| `--tags,-t <dir>` | Tags directory. Use multiple times to add tags directories. |

### invader-font
This program generates font tags.

**Usage:** `invader-font [options] <font-tag>`

| Option                | Description                                |
| --------------------- | ------------------------------------------ |
| `--help,-h`           | Show help.                                 |
| `--info,-i`           | Show credits, source info, and other info. |
| `--fs-path,-P`        | Use a filesystem path for the font data.   |
| `--data,-d <dir>`     | Data directory.                            |
| `--maps,-m <dir>`     | Maps directory.                            |
| `--font-size,-s <px>` | Use a font size in pixels.                 |

### invader-indexer
This program builds index files for usage with `--with-index` with invader-build.

**Usage:** `invader-indexer <input map> <output index>`

### invader-resource
This program builds resource maps. Only maps with stock tags can be built. These files are not guaranteed to work with
existing cache files.

**Usage:** `invader-resource <options>`

| Option             | Description                                                        |
| ------------------ | ------------------------------------------------------------------ |
| `--info,-i`        | Show credits, source info, and other info.                         |
| `--help,-h`        | Show help.                                                         |
| `--maps,-m <dir>`  | Use a specific maps directory.                                     |
| `--tags,-t <dir>`  | Tags directory. Use multiple times to add tags directories.        |
| `--type,-T <type>` | Set resource map (required). Can be `bitmaps`, `sounds`, or `loc`. |
| `--retail,-R`      | Build a retail/demo resource map.                                  |

## Frequently Asked Questions
These are a selection of questions that I've received over the course of Invader's development.

- [What operating systems are supported?](#what-operating-systems-are-supported)
- [How can I contribute?](#how-can-i-contribute)
- [Can invader-build create .yelo maps?](#can-invader-build-create-yelo-maps)
- [Can invader-build create Xbox maps?](#can-invader-build-create-xbox-maps)
- [The HEK says my bitmap tag is "too large" when opening.](#the-hek-says-my-bitmap-tag-is-too-large-when-opening)
- [How close to completion is Invader?](#how-close-to-completion-is-invader)
- [Should I use invader-bitmap or tool.exe?](#should-i-use-invader-bitmap-or-toolexe)
- [Should I use invader-build or tool.exe?](#should-i-use-invader-build-or-toolexe)

### What operating systems are supported?
Invader does not support any one operating system.

### How can I contribute?
That information is in CONTRIBUTING.md.

### Can invader-build create .yelo maps?
Officially, invader-build only creates maps for the Gearbox port of Halo on the PC. The .yelo file format is specific
to Open Sauce, a mod of Halo Custom Edition. Therefore, the Invader project does not support it. However, this does not
mean that you can't make a fork of Invader that supports it, and there are people who have said they were willing to do
this.

### Can invader-build create Xbox maps?
Officially, invader-build only creates maps for the Gearbox port of Halo on the PC. While Xbox maps are very similar
in format to PC maps, there exists enough differences to make supporting the Xbox version non-trivial. Kavawuvi also
does not have a modded Xbox or a retail copy of the Xbox version of the game, so there is no means to debug or test.

### The HEK says my bitmap tag is "too large" when opening.
The HEK has a 16 MiB limitation for bitmap tags. Invader does not have this limitation, and you can use the MEK to view
bitmap tags that exceed 16 MiB. Halo PC also does not have any problems loading bitmaps that exceed 16 MiB. That said,
some DirectX 9 GPUs and/or implementations won't support textures larger than 2048x2048 (2D textures) or 256x256x256
(3D textures).

It is worth noting that invader-build and various other tools let you specify multiple tag directories. If you need
to use Sapien, you can put a lower quality, compressed version of your bitmap in your main tags folder, and you can put
the higher quality version in a tags directory that takes priority. Sapien will use the lower quality bitmap and load
happily, and invader-build will use the higher quality bitmap when building.

### How close to completion is Invader?
There is still a lot to do in Invader. Check the Issues page for more information.

### Should I use invader-bitmap or tool.exe?
In this case, invader-bitmap is either mostly or completely feature-complete, and it has a number of features tool.exe
does not have, such as support for the .tga format (goes well with Refinery's data extraction) as well as dithering.
Therefore, invader-bitmap is the superior choice.

### Should I use invader-build or tool.exe?
It depends. Right now, invader-build isn't finished and has a number of issues that need ironed out before this FAQ can
confidently recommend it for *every* use case, while tool.exe is basically the reference when it comes to building
cache files.

Most singleplayer maps will probably work, but until the stock campaign works as it did when built with tool.exe, then
it is not recommended to use invader-build for singleplayer maps except for testing.

Most multiplayer maps work fine when built under invader-build. Because tool.exe has a number of bugs and invader-build
does quite a few things tool.exe doesn't do, there are a few reasons to use invader-build over tool.exe for multiplayer
maps.
