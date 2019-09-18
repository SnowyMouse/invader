# Invader
Invader is an open source toolkit for creating Halo: Combat Evolved maps. It is written in C++ using the C++17 standard.

## Building Invader
There are a number of things you will need in order to successfully compile Invader.

### Required Dependencies
* C++17 compiler
* C99 compiler
* CMake 3.10 or newer
* Python 3.7 or newer

### Optional Dependencies
* LibArchive (invader-archive)
* LibTIFF (invader-bitmap)
* zlib (invader-bitmap)
* freetype (invader-font)
* git (git commit hash in version - build only)

### Windows
To compile on Windows, you will need to install 64-bit [MSYS2](https://www.msys2.org/), installing the aforementioned
software (Python, etc.) through MSYS2 and NOT through their Windows installers. Ensure you are using the MSYS2 MinGW
64-bit shell when building.

MSVC is unsupported and will not work.

### Compiling
First, you will need to download the Invader repository onto your computer. You can do this using the command:
```
git clone https://github.com/Kavawuvi/Invader
```

Everything from this point on assumes the Invader repository was cloned in a directory called "Invader" in the
current directory.

Next, you will need to create an out-of-source build directory. You can use this command to make the build
directory:
```
mkdir invader-build && cd invader-build
```

Next, use this command to set up the CMake build directory if you're on Linux:
```
cmake ../Invader -D CMAKE_BUILD_TYPE=Release
```

If you're building this with MSYS (Windows), use this, instead:
```
cmake ../Invader -D CMAKE_BUILD_TYPE=Release -G "MSYS Makefiles"
```

Lastly, you can compile this using the `make` command, optionally using `-j#` to specify the maximum number of jobs if
you want to build using multiple processing threads.

## Programs
To remove the reliance of one huge executable, something that has caused issues with Halo Custom Edition's tool.exe, as
well as make things easier for me to work with, this project is split into different programs.

### invader-archive
This program generates a .tar.xz archive containing all of the tags used to build a map.

#### Usage
`invader-archive [options] <scenario-tag | -s tag.class>`

| Option | Description |
| --- | --- |
| `--info,-i` | Show credits, source info, and other info. |
| `--help,-h` | Show help. |
| `--output,-o <file>` | Output to a specific file. Extension must be .tar.xz. |
| `--single-tag,-s` | Archive a tag tree instead of a cache file tree. |
| `--tags,-t <dir>` | Tags directory. Use multiple times to add tags directories. |

### invader-bitmap
This program generates bitmap tags from images. For source images, .tif, .tiff, .png, .tga, and .bmp extensions are
supported.

#### Usage
`invader-bitmap [options] <bitmap-tag>`

| Option | Description |
| --- | --- |
| `--info,-i` | Show credits, source info, and other info. |
| `--help,-h` | Show help. |
| `--data,-d <dir>` | Data directory. |
| `--tags,-t <dir>` | Tags directory. |
| `--type,-T` | Set bitmap type. Can be `2d` (default), `3d`, `cubemap`, `interface`, or `sprite`. |
| `--usage,-u <usage>` | Set usage. Can be `default` (default) or `bumpmap`. |
| `--dithering,-D <channels>` | Apply dithering for dxt, 16-bit, and p8. Specify channels with letters (i.e. `argb`). |
| `--ignore-tag,-I` | Ignore the tag data if the tag exists. |
| `--format,-F <format>` | Pixel format. Can be `32-bit` (default), `16-bit`, `monochrome`, `dxt1`, `dxt3`, or `dxt5`. |
| `--mipmap-count,-m <count>` | Set maximum mipmap count. By default, this is 32767. |
| `--mipmap-scale,-s <type>` | Mipmap scaling. Can be `linear` (default), `nearest-alpha`, `nearest`, or `none`. |
| `--detail-fade,-f <factor>` | Set fade-to-gray factor for mipmaps of detail maps. By default, this is 0. |
| `--spacing,-S <px>` | Set minimum spacing between sprites in pixels. By default, this is 4. |
| `--bump-height,-H <height>` | Set apparent bump height from 0 to 1. By default, this is 0.02. |
| `--budget-count,-C <count>` | Set max number of sprite sheets. 0 (default) disables budgeting. |
| `--budget,-B <length>` | Set max length of sprite sheet. By default, this is 32. |

#### Uncompressed Bitmap Formats
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

#### Block-Compressed Bitmap Formats
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

#### More Formats
These formats were originally available on Xbox and do not work on stock Halo PC without a mod (e.g.
[Chimera](https://github.com/Kavawuvi/Chimera)). They all use explicit RGB and/or alpha.

If you use monochrome with a monochrome bitmap used as input, then there will be no loss in quality.

| Format       | Storage  | Bits/px | Alpha   | RGB     | Notes                                       |
| ------------ | -------- | ------- | ------- | ------- | ------------------------------------------- |
| `monochrome` | A8Y8     | 16      | 8-bit   | 8-bit   | Intensity (R=G=B)                           |
|              | A8       | 8       | 8-bit   |         | All pixels 100% intensity                   |
|              | Y8       | 8       |         | 8-bit   | All pixels 100% alpha                       |
| palettized   | P8       | 8       | Indexed | Indexed | Bumpmaps only; based on palette from Stubbs |

### invader-build
This program builds cache files.

#### Usage
`invader-build [options] <scenario-tag>`

| Option | Description |
| --- | --- |
| `--info,-i` | Show credits, source info, and other info. |
| `--help,-h` | Show help. |
| `--maps,-m <dir>` | Use a specific maps directory. |
| `--always-index-tags,-a` | Always index tags with resource maps when possible. |
| `--no-indexed-tags,-n` | Do not index tags with resource maps. |
| `--forge-crc,-c <crc>` | Forge the CRC. |
| `--output,-o <file>` | Output to a specific file. |
| `--quiet,-q` | Only output error messages. |
| `--tags,-t <dir>` | Tags directory. Use multiple times to add tags directories. |
| `--with-index,-w <file>` | Use an index file for the tags. |

### invader-crc
This program calculates the CRC32 of a map file.

#### Usage
`invader-crc <map>`

### invader-dependency
This program finds tags that directly depend on a given tag.

#### Usage
`invader-dependency [options] <tag.class>`

| Option | Description |
| --- | --- |
| `--help,-h` | Show help. |
| `--info,-i` | Show credits, source info, and other info. |
| `--recursive,-R` | Recursively get all depended tags. |
| `--reverse,-r` | Find all tags that depend on the tag, instead. |
| `--tags,-t <dir>` | Tags directory. Use multiple times to add tags directories. |

### invader-font
This program generates font tags.

#### Usage
`invader-font [options] <font-tag>`

| Option | Description |
| --- | --- |
| `--help,-h` | Show help. |
| `--info,-i` | Show credits, source info, and other info. |
| `--data,-d <dir>` | Data directory. |
| `--maps,-m <dir>` | Maps directory. |
| `--font-size,-s <px>` | Use a font size in pixels. |

### invader-indexer
This program builds index files for usage with `--with-index` with invader-build.

#### Usage
`invader-indexer <input map> <output index>`

### invader-resource
This program builds resource maps.

#### Usage
`invader-resource <options>`

| Option | Description |
| --- | --- |
| `--info,-i` | Show credits, source info, and other info. |
| `--help,-h` | Show help. |
| `--maps,-m <dir>` | Use a specific maps directory. |
| `--tags,-t <dir>` | Tags directory. Use multiple times to add tags directories. |
| `--type,-T <type>` | Set resource map (required). Can be `bitmaps`, `sounds`, or `loc`. |
