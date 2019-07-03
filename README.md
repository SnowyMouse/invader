# Invader

Invader is an open source map and tag builder for Halo: Custom Edition written in C++ using the C++17 standard.

## Programs

To remove the reliance of one huge executable, something that has caused issues with Halo Custom Edition's tool.exe, as
well as make things easier for me to work with, this project is split into different programs.

### invader-bitmap

This program generates bitmap files. It takes the following arguments:

| Argument | Alternate | Description |
| --- | --- | --- |
| `--info` | `-i` | Show credits, source info, and other info. |
| `--data <dir>` | `-d` | Set data directory. |
| `--tags <dir>` | `-t` | Set tags directory. |
| `--input-format <format>` | `-I` | Set input format. Can be `tif` (default) or `png`. |
| `--mipmap-fade <factor>` | `-f` | Set fade-to-gray factor for mipmaps. |
| `--mipmap-scale <type>` | `-s` | Set mipmap scale type. Can be `linear` (default, interpolate colors), `nearest-alpha` (interpolate alpha only), `nearest` (do not interpolate colors), `none` (do not generate mipmaps). |

### invader-build

This program builds cache files.

| Argument | Alternate | Description |
| --- | --- | --- |
| `--info` | `-i` | Show credits, source info, and other info. |
| `--maps <dir>` | `-m` | Use a specific maps directory. |
| `--no-indexed-tags` | `-n` | Do not index tags. This can speed up build time at the cost of a much larger file size. |
| `--output <file>` | `-o` | Output to a specific file. |
| `--quiet` | `-q` | Only output error messages. |
| `--tags <dir>` | `-t` | Set tags directory. Use multiple times to add tags directories ordered by precedence. |
| `--with-index <file>` | `-w` | Use an index file for the tags, ensuring the map's tags are ordered in the same way. |

### invader-crc

This program gets and/or modifies the calculated CRC32 of a map file. It takes one or two arguments: `<map> [new crc]`.
Not specifying a CRC32 value will only calculate the CRC32 value of the cache file without modifying it. Otherwise, it
will modify it and then calculate the CRC32.

### invader-indexer

This program builds index files for usage with `--with-index` with invader-build. It takes exactly two arguments:
`<input map> <output index>`
