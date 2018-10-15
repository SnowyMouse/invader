# Invader

Invader is an open source map and tag builder for Halo: Custom Edition written in C++.

In order to build this project, your compiler must support the C++17 standard. GCC version 7.3 or newer has been tested and works with this, though I do plan on implementing std::filesystem - something that is only supported in GCC 8.2 or newer.

This project is current separated into two programs: invader-build and invader-indexer.

## invader-build

This program builds cache files. The command line arguments are as follows:

| Argument | Alternate | Description |
| --- | --- | --- |
| `--info` | `-i` | Show credits, source info, and other info. |
| `--maps <dir>` | `-m` | Use a specific maps directory. |
| `--no-indexed-tags` | `-n` | Do not index tags. This can speed up build time at the cost of a much larger file size. |
| `--output <file>` | `-o` | Output to a specific file. |
| `--quiet` | `-q` | Only output error messages. |
| `--tags [dir] [...]` | `-t` | Use the specified tags directory(s). Specify directories in order of precedence. |
| `--with-index <file>` | `-w` | Use an index file for the tags, ensuring the map's tags are ordered in the same way. |

## invader-indexer

This program builds index files for usage with `--with-index` with Invader.
