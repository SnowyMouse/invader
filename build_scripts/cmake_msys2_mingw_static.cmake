# Pre-run
if(INVADER_BUILD_DEPENDENCY_SCRIPT_PRE_RUN)
    # For Qt6
    set(INVADER_USING_WIN32_STATIC_QT ON)

    # Invader should obviously not build with shared libs
    set(BUILD_SHARED_LIBS OFF)

    # Set these
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++ -lwinpthread")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFLAC__NO_DLL")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFLAC__NO_DLL")

    # prefer static libraries
    list(APPEND PKG_CONFIG_EXECUTABLE "--static")

    # disable use of find modules that don't work for static libraries
    set(CMAKE_DISABLE_FIND_PACKAGE_harfbuzz TRUE)

    set(FREETYPE_DEPENDENCIES "-lbz2;-lharfbuzz;-lfreetype;-lbrotlidec;-lbrotlicommon;-lrpcrt4" CACHE INTERNAL "dependencies of static FreeType2 library")
    set(HARFBUZZ_DEPENDENCIES "-lglib-2.0;${GLIB2_DEPENDENCIES};-lintl;-lm;-lfreetype;-lgraphite2" CACHE INTERNAL "dependencies of static HarfBuzz library")

    # Replace ".dll.a" with ".a"
    # Changing the lookup suffix does not work, because ".a" still matches ".dll.a"
    # So because some smartass years ago decided to call them ".dll.a" we have to do this disgusting shit.
    set(LIBB2_LIBRARY                   "${INVADER_MINGW_PREFIX}/lib/libb2.a")
    set(BrotliCommon_LIBRARY_DEBUG      "${INVADER_MINGW_PREFIX}/lib/libbrotlicommon.a")
    set(BrotliCommon_LIBRARY_RELEASE    "${INVADER_MINGW_PREFIX}/lib/libbrotlicommon.a")
    set(BrotliDec_LIBRARY_DEBUG         "${INVADER_MINGW_PREFIX}/lib/libbrotlidec.a")
    set(BrotliDec_LIBRARY_RELEASE       "${INVADER_MINGW_PREFIX}/lib/libbrotlidec.a")
    set(BrotliEnc_LIBRARY_DEBUG         "${INVADER_MINGW_PREFIX}/lib/libbrotlienc.a")
    set(BrotliEnc_LIBRARY_RELEASE       "${INVADER_MINGW_PREFIX}/lib/libbrotlienc.a")
    set(FREETYPE_LIBRARY_RELEASE        "${INVADER_MINGW_PREFIX}/lib/libfreetype.a")
    set(HARFBUZZ_LIBRARIES              "${INVADER_MINGW_PREFIX}/lib/libharfbuzz.a")
    set(JPEG_LIBRARY_RELEASE            "${INVADER_MINGW_PREFIX}/lib/libjpeg.a")
    set(LIB_EAY                         "${INVADER_MINGW_PREFIX}/lib/libcrypto.a")
    set(LibArchive_LIBRARY              "${INVADER_MINGW_PREFIX}/lib/libarchive.a")
    set(PCRE2_LIBRARY_DEBUG             "${INVADER_MINGW_PREFIX}/lib/libpcre2-16.a")
    set(PCRE2_LIBRARY_RELEASE           "${INVADER_MINGW_PREFIX}/lib/libpcre2-16.a")
    set(PNG_LIBRARY_RELEASE             "${INVADER_MINGW_PREFIX}/lib/libpng.a")
    set(SSL_EAY                         "${INVADER_MINGW_PREFIX}/lib/libssl.a")
    set(TIFF_LIBRARY_RELEASE            "${INVADER_MINGW_PREFIX}/lib/libtiff.a")
    set(WebP_LIBRARY                    "${INVADER_MINGW_PREFIX}/lib/libwebp.a")
    set(WebP_demux_LIBRARY              "${INVADER_MINGW_PREFIX}/lib/libwebpdemux.a")
    set(WebP_mux_LIBRARY                "${INVADER_MINGW_PREFIX}/lib/libwebpmux.a")
    set(ZLIB_LIBRARY_RELEASE            "${INVADER_MINGW_PREFIX}/lib/libz.a")

# Fix any dependencies needed to be fixed
else()
    set(DEP_SQUISH_LIBRARIES squish gomp)
    set(SDL2_LIBRARIES ${SDL2_STATIC_LIBRARIES} -lsetupapi -limm32 -lversion -lwinmm)
    set(TIFF_LIBRARIES ${TIFF_LIBRARIES} deflate lzma zstd jpeg jbig webp sharpyuv Lerc)
    set(LibArchive_LIBRARIES ${LibArchive_LIBRARIES} lzma zstd bz2 iconv crypto crypt32 bcrypt)
    set(FREETYPE_LIBRARIES freetype png brotlidec brotlicommon bz2 harfbuzz graphite2 freetype dwrite rpcrt4)
endif()
