# Invader

Invader is an open source map and tag builder for Halo: Custom Edition written in C++ using the C++17 standard.

## Programs

To remove the reliance of one huge executable, something that has caused issues with Halo Custom Edition's tool.exe, as
well as make things easier for me to work with, this project is split into different programs.

### invader-build

This program builds cache files.

| Argument | Alternate | Description |
| --- | --- | --- |
| `--info` | `-i` | Show credits, source info, and other info. |
| `--maps <dir>` | `-m` | Use a specific maps directory. |
| `--no-indexed-tags` | `-n` | Do not index tags. This can speed up build time at the cost of a much larger file size. |
| `--output <file>` | `-o` | Output to a specific file. |
| `--quiet` | `-q` | Only output error messages. |
| `--tags [dir] [...]` | `-t` | Use the specified tags directory(s). Specify directories in order of precedence. |
| `--with-index <file>` | `-w` | Use an index file for the tags, ensuring the map's tags are ordered in the same way. |

### invader-indexer

This program builds index files for usage with `--with-index` with invader-build. It takes exactly two arguments:
`<input map> <output index>`
