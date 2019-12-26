# Invader
Invader is an open source toolkit for creating maps and assets for Halo: Combat
Evolved on the PC. See LICENSE for more information about the license.

Our Discord server is https://discord.gg/RCX3nvw

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
- C++17 compiler with support for `filesystem`
- C99 compiler
- CMake 3.12 or newer
- Python 3.7 or newer
- Zstandard 1.3 or newer
- LibTIFF 3.6 or newer
- libvorbis 1.3.6 or newer
- libsamplerate 0.1.9 or newer

##### Optional dependencies
- LibArchive ([invader-archive])
- zlib ([invader-bitmap])
- freetype ([invader-font])
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
with Halo Custom Edition's tool.exe, as well as make things easier to develop,
this project is split into different programs.
- [invader-archive]
- [invader-bitmap]
    - [Uncompressed bitmap formats]
    - [Block-compressed bitmap formats]
    - [More bitmap formats]
    - [Which bitmap format should I use?]
- [invader-build]
- [invader-compress]
- [invader-dependency]
- [invader-extract]
- [invader-font]
- [invader-indexer]
- [invader-info]
- [invader-resource]
- [invader-sound]
    - [What is splitting?]
    - [Audio formats]
    - [Which audio format should I use?]

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
                               Default (new tag): 0.026
  -i --info                    Show license and credits.
  -I --ignore-tag              Ignore the tag data if the tag exists.
  -M --mipmap-count <count>    Set maximum mipmaps. Default (new tag): 32767
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
and a bitmap does not have transparency, then dxt1 will automatically be used
to keep the size as small as possible.

We do not recommend using these formats. They are only provided for completion
sake.

Format | Bits/px | Alpha              | Notes
------ | ------- | ------------------ | --------------------------------------
DXT1   | 4       |                    | 100% alpha
DXT3   | 8       | 4-bit explicit     | Better for shapes like HUDs
DXT5   | 8       | 4-bit interpolated | Better for alpha gradients like clouds

#### More bitmap formats
These formats were originally available on Xbox and do not work on stock Halo.
They all use explicit RGB and/or alpha.

If you use monochrome with a monochrome bitmap used as input, then there will
be no loss in quality.

Format       | Storage  | Bits/px | Alpha   | RGB     | Notes
------------ | -------- | ------- | ------- | ------- | ---------------------
monochrome   | A8Y8     | 16      | 8-bit   | 8-bit   | Intensity (R=G=B)
monochrome   | A8       | 8       | 8-bit   |         | 100% intensity
monochrome   | Y8       | 8       |         | 8-bit   | 100% alpha
palettized   | P8       | 8       | Indexed | Indexed | Bump compression only

#### Which bitmap format should I use?
32-bit color and monochrome (with monochrome input) have zero quality loss.
Unfortunately, Halo PC does not support monochrome, and it'd easily be the best
choice for monochrome textures like HUD masks. So, the best format with zero
loss in quality is 32-bit color. Obviously, this results in a larger file size,
with a 1024x1024 bitmap taking up ~5.3 MiB and a 2048x2048 bitmap taking up
~21.3 MiB, with mipmaps included for both. But if you don't care, this is the
best way to go.

16-bit color works well on some textures without any noticeable impact, such as
noise maps and some simple textures, but it is subject to banding due to a lack
of color depth. The color depth also gets significantly worse if you use an
alpha channel.

dxt1 takes up very little space, but make no mistake: it's a lossy compression
format, and the image will lose finer details as a result - an issue that
16-bit often does not have. As such, it's quite bad on HUDs, detail maps,
bumpmaps, and multipurpose maps, and, in our opinion, literally anything else.

dxt3 and dxt5 use dxt1 for color, only differing in how alpha works. Alpha is
explicit on dxt3, thus dxt3 works better for things that need definite shape
such as HUD backgrounds. It will result in banding on gradients, however. dxt5
compresses alpha to be interpolated like the color, so it works better for
more things than dxt3.

### invader-build
This program builds cache files.

```
Usage: invader-build [options] <scenario>

Build cache files for Halo Combat Evolved on the PC.

Options:
  -a --always-index-tags       Always index tags when possible. This can speed
                               up build time, but stock tags can't be modified.
  -c --compress                Compress the cache file.
  -C --forge-crc <crc>         Forge the CRC32 value of the map after building
                               it.
  -g --game-engine <id>        Specify the game engine. Valid engines are:
                               custom (default), retail, demo, dark
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -m --maps <dir>              Use a specific maps directory.
  -n --no-external-tags        Do not use external tags. This can speed up
                               build time at a cost of a much larger file size.
  -N --rename-scenario <name>  Rename the scenario.
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
                               19. Default: 19
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

### invader-extract
This program extracts tags from cache files.

```
Usage: invader-extract [options] <map>

Extract data from cache files.

Options:
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info
  -m --maps <dir>              Set the maps directory
  -n --non-mp-globals          Enable extraction of non-multiplayer .globals
  -O --overwrite               Overwrite tags if they already exist
  -r --recursive               Extract tag dependencies
  -s --search <expr>           Search for tags (* and ? are wildcards); use
                               multiple times for multiple queries
  -t --tags <dir>              Set the tags directory
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

### invader-indexer
This program builds index files for usage with `--with-index` with invader-build.

```
Usage: invader-indexer [options] <input-map> <output-txt>

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
  -T --type <type>             Set the type of data to show. Can be overview
                               (default), build, compressed, compression-ratio,
                               crc32, crc32-mismatched, dirty, engine,
                               protected, map-type, scenario, scenario-path,
                               tag-count, tags
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
  -c --class                   Set the class. This is required when generating
                               new sounds. Can be: ambient-computers,
                               ambient-machinery, ambient-nature,
                               device-computers, device-door,
                               device-force-field, device-machinery,
                               device-nature, first-person-damage, game-event,
                               music, object-impacts, particle-impacts,
                               projectile-impact, projectile-detonation,
                               scripted-dialog-force-unspatialized,
                               scripted-dialog-other, scripted-dialog-player,
                               scripted-effect, slow-particle-impacts,
                               unit-dialog, unit-footsteps, vehicle-collision,
                               vehicle-engine, weapon-charge, weapon-empty,
                               weapon-fire, weapon-idle, weapon-overheat,
                               weapon-ready, weapon-reload
  -C --channel-count           Set the channel count. Can be: mono, stereo. By
                               default, this is determined based on the input
                               audio.
  -d --data <dir>              Use the specified data directory.
  -F --format                  Set the format. Can be: 16-bit-pcm, ogg-vorbis,
                               xbox-adpcm. Default (new tag): 16-bit-pcm
  -h --help                    Show this list of options.
  -i --info                    Show credits, source info, and other info.
  -P --fs-path                 Use a filesystem path for the data.
  -q --vorbis-quality          Set the Vorbis quality. This can be between -0.1
                               and 1.0. Default: 1.0
  -r --sample-rate             Set the sample rate in Hz. Halo supports 22050
                               and 44100. By default, this is determined based
                               on the input audio.
  -s --split                   Split permutations into 227.5 KiB chunks.
  -S --no-split                Do not split permutations.
  -t --tags <dir>              Use the specified tags directory. Use multiple
                               times to add more directories, ordered by
                               precedence.
```

#### What is splitting?
The Halo engine was written to primarily handle short-length sounds. It cannot
handle extremely long audio sections due to memory and engine limitations, and
this is regardless of the audio format being used since all audio is eventually
handled as a 16-bit PCM stream.

Stock tool.exe splits Ogg Vorbis audio into 910 KiB chunks and all other audio
to 227.5 KiB chunks before compression, but we found that looping issues were
more likely to occur with the higher chunk size. Therefore, all splitting is
done on 227.5 KiB with invader-sound.

For sounds that are more than 2.5 seconds long if 44100 Hz stereo, 5 seconds if
22050 Hz stereo, or 10 seconds long if 22050 Hz mono, such as music or extended
dialogue, we recommend enabling splitting.

#### Audio formats
These are the different audio formats that invader-sound supports.

Format           | Bitrate (44100 Hz stereo) | Type
---------------- | ------------------------- | --------------------------------
16-bit PCM       | 1411.2 kbps               | Lossless unless input is >16-bit
Ogg Vorbis (1.0) | ~435.8 kbps (varies)      | Max quality, lossy
Ogg Vorbis (0.5) | ~142.7 kbps (varies)      | "Transparent" quality, lossy
Ogg Vorbis (0.3) | ~105.6 kbps (varies)      | Oggenc's default quality, lossy
Xbox ADPCM       | ~390.8 kbps               | Lossy, fixed bitrate

#### Which audio format should I use?
The only lossless format available is 16-bit PCM. This will, however, result in
a drastic increase in map size, so it is not recommended to use this with long
sounds.

Ogg Vorbis provides a good tradeoff in terms of bitrate and quality. Using 0.5
is considered "transparent" provided you use a lossless audio input (if not,
then you may need to use a higher quality value), and this also gives you lower
bitrate and better quality than Xbox ADPCM.

Xbox ADPCM does not compress as efficiently as Ogg Vorbis, but it decodes
considerably faster, so this may be beneficial for firing effects. However, we
instead recommend using uncompressed 16-bit PCM if possible.

## Frequently asked questions
These are a selection of questions that have been asked over the course of
Invader's development.
- [What operating systems are supported?]
- [Why GPL and not MIT or BSD?]
- [Are there any GUI tools?]
- [Are 32-bit Windows builds available?]
- [Can invader-build create .yelo maps?]
- [Can invader-build create Xbox maps?]
- [The HEK says my bitmap tag is "too large" when opening.]
- [How close to completion is Invader?]
- [Should I use invader-bitmap or tool.exe?]
- [Should I use invader-build or tool.exe?]

### What operating systems are supported?
Invader does not support any one operating system, but Invader is developed on
Arch Linux, and 64-bit Windows builds are available via [Nightly Builds].

### Why GPL and not MIT or BSD?
The Halo CE modding community has a habit of obfuscating knowledge that could
be used to help other people. This is often done through these means:
- [Map protection]
- [Writing closed source software]
- [Obfuscating software]
- [Keeping helpful information locked behind secret forums and groups]

Often times, people will say they will release the source code in the future,
or they will at least consider releasing the source code. In nearly all cases
of this, the source code was never released. In fact, many great
accomplishments have been lost to time, as most of the people who made them are
no longer there to support what they have made.

As others are forced to rely on this software, then over time, people realize
how broken, incomplete, or even incompatible the software is. For example, the
Halo Editing Kit has numerous issues that prevent people from making the
content they want to make, yet [it is still the only way to fully create maps].
As a result, making closed source software does nothing but harm for any sort
of community.

In our opinion, having a requirement that people give back to the community
any changes to Invader that they have released is not too much to ask for. That
is why Invader is GPL.

### Are there any GUI tools?
Officially, none of these tools currently have graphical user interfaces. Some
people have offered to make GUI versions of these tools, and it probably isn't
difficult to make a GUI wrapper for these tools due to the nature of them.

There are a few reasons why Invader officially has only command-line tools:
- Command-line tools require significantly less time to write and test.
- Command-line tools require fewer dependencies (e.g. no Qt or GTK).
- Command-line tools work well with scripts and shell commands.
- Command-line shells are very optimized at quick and precise execution,
  providing features such as tab completion and command history.
- Invader tools usually perform exactly one task: take a small amount of input
  and turn it into an output. A GUI will likely make such a simple task slower.

Basically, for most functions, a command-line interface is enough, while a GUI
may add overhead to such a task (e.g. mouse usage, file navigation, etc.) while
not getting any of the benefits of a command-line shell such as tab completion
or command history.

Even so, there are some functions where a graphical user interface is better.
Tasks that require a large amount of user interaction such as direct editing of
HEK tag files or scenario editing are tasks that are better suited to a GUI
than the command line, as opposed to simple tasks such as building a cache file
which requires a small amount of input to perform the entire task.

### Are 32-bit Windows builds available?
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
[Why GPL and not MIT or BSD?]: #why-gpl-and-not-mit-or-bsd
[Are there any GUI tools?]: #are-there-any-gui-tools
[Are 32-bit Windows builds available?]: #are-32-bit-windows-builds-available
[Can invader-build create .yelo maps?]: #can-invader-build-create-yelo-maps
[Can invader-build create Xbox maps?]: #can-invader-build-create-xbox-maps
[The HEK says my bitmap tag is "too large" when opening.]: #the-hek-says-my-bitmap-tag-is-too-large-when-opening
[How close to completion is Invader?]: #how-close-to-completion-is-invader
[Should I use invader-bitmap or tool.exe?]: #should-i-use-invader-bitmap-or-toolexe
[Should I use invader-build or tool.exe?]: #should-i-use-invader-build-or-toolexe

[Map protection]: http://forum.halomaps.org/index.cfm?page=topic&topicID=16885
[Obfuscating software]: http://forum.halomaps.org/index.cfm?page=topic&topicID=51196&start=112
[Writing closed source software]: http://halo.isimaginary.com/
[Keeping helpful information locked behind secret forums and groups]: http://www.macgamingmods.com/forum/viewtopic.php?f=40&t=7948
[it is still the only way to fully create maps]: https://opencarnage.net/index.php?/topic/7765-replacing-the-halo-editing-kit-with-open-source-software/

[invader-archive]: #invader-archive
[invader-bitmap]: #invader-bitmap
[invader-build]: #invader-build
[invader-compress]: #invader-compress
[invader-dependency]: #invader-dependency
[invader-extract]: #invader-extract
[invader-font]: #invader-font
[invader-indexer]: #invader-indexer
[invader-info]: #invader-info
[invader-resource]: #invader-resource
[invader-sound]: #invader-sound

[Uncompressed bitmap formats]: #uncompressed-bitmap-formats
[Block-compressed bitmap formats]: #block-compressed-bitmap-formats
[More bitmap formats]: #more-bitmap-formats
[Which bitmap format should I use?]: #which-bitmap-formats-should-i-use
[What is splitting?]: #what-is-splitting
[Audio formats]: #audio-formats
[Which audio format should I use?]: #which-audio-format-should-i-use
