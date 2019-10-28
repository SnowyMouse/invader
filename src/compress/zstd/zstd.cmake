# SPDX-License-Identifier: GPL-3.0-only

# zstd library
add_library(invader-zstd STATIC
    src/compress/zstd/lib/dictBuilder/zdict.c
    src/compress/zstd/lib/dictBuilder/fastcover.c
    src/compress/zstd/lib/dictBuilder/cover.c
    src/compress/zstd/lib/dictBuilder/divsufsort.c
    src/compress/zstd/lib/deprecated/zbuff_compress.c
    src/compress/zstd/lib/deprecated/zbuff_common.c
    src/compress/zstd/lib/deprecated/zbuff_decompress.c
    src/compress/zstd/lib/compress/zstd_compress.c
    src/compress/zstd/lib/compress/zstd_compress_sequences.c
    src/compress/zstd/lib/compress/zstd_lazy.c
    src/compress/zstd/lib/compress/fse_compress.c
    src/compress/zstd/lib/compress/zstd_fast.c
    src/compress/zstd/lib/compress/hist.c
    src/compress/zstd/lib/compress/zstdmt_compress.c
    src/compress/zstd/lib/compress/zstd_opt.c
    src/compress/zstd/lib/compress/zstd_double_fast.c
    src/compress/zstd/lib/compress/zstd_ldm.c
    src/compress/zstd/lib/compress/zstd_compress_literals.c
    src/compress/zstd/lib/compress/huf_compress.c
    src/compress/zstd/lib/common/debug.c
    src/compress/zstd/lib/common/pool.c
    src/compress/zstd/lib/common/fse_decompress.c
    src/compress/zstd/lib/common/xxhash.c
    src/compress/zstd/lib/common/threading.c
    src/compress/zstd/lib/common/entropy_common.c
    src/compress/zstd/lib/common/zstd_common.c
    src/compress/zstd/lib/common/error_private.c
    src/compress/zstd/lib/legacy/zstd_v02.c
    src/compress/zstd/lib/legacy/zstd_v06.c
    src/compress/zstd/lib/legacy/zstd_v01.c
    src/compress/zstd/lib/legacy/zstd_v03.c
    src/compress/zstd/lib/legacy/zstd_v04.c
    src/compress/zstd/lib/legacy/zstd_v05.c
    src/compress/zstd/lib/legacy/zstd_v07.c
    src/compress/zstd/lib/decompress/zstd_decompress.c
    src/compress/zstd/lib/decompress/zstd_decompress_block.c
    src/compress/zstd/lib/decompress/zstd_ddict.c
    src/compress/zstd/lib/decompress/huf_decompress.c
)

target_include_directories(invader-zstd
    PUBLIC src/compress/zstd/lib/common
    PUBLIC src/compress/zstd/lib
)
