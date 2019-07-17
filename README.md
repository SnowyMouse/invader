# Invader
Invader is an open source map and tag builder for Halo: Custom Edition written in C++ using the C++17 standard.

## Programs
To remove the reliance of one huge executable, something that has caused issues with Halo Custom Edition's tool.exe, as
well as make things easier for me to work with, this project is split into different programs.

### invader-bitmap
This program generates bitmap files.

#### Usage
`invader-bitmap [options] <bitmap-tag>`

| Option | Description |
| --- | --- |
| `--info,-i` | Show credits, source info, and other info. |
| `--help,-h` | Show help. |
| `--data,-d <dir>` | Data directory. |
| `--tags,-t <dir>` | Tags directory. |
| `--dithering,-D` | Apply dithering. Only works on `dxt1`, `dxt3`, and `dxt5` for now. |
| `--ignore-tag,-I` | Ignore the tag data if the tag exists. |
| `--format,-F <format>` | Pixel format. Can be `32-bit` (default), `16-bit`, `monochrome`, `dxt1`, `dxt3`, or `dxt5`. |
| `--mipmap-count,-m <count>` | Set maximum mipmap count. Negative numbers discard `<count>` mipmaps, instead. |
| `--mipmap-fade,-f <factor>` | Fade-to-gray factor for mipmaps. |
| `--mipmap-scale,-s <type>` | Mipmap scaling. Can be `linear` (default), `nearest-alpha`, `nearest`, or `none`. |

#### Requirements
* libtiff
* libzlib

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
| `--output,-o <file>` | Output to a specific file. |
| `--quiet,-q` | Only output error messages. |
| `--tags,-t <dir>` | Tags directory. Use multiple times to add tags directories. |
| `--with-index,-w <file>` | Use an index file for the tags. |

### invader-crc
This program gets and/or modifies the calculated CRC32 of a map file.

#### Usage
`invader-crc <map> [new crc]`

Not specifying a CRC32 value will only calculate the CRC32 value of the cache file without modifying it. Otherwise, it
will modify it and then calculate the CRC32.

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

#### Requirements
* Python 3 (build only)
