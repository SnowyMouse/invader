# Pre-run
if(INVADER_BUILD_DEPENDENCY_SCRIPT_PRE_RUN)
    # For Qt6 and brotli
    set(INVADER_USING_WIN32_STATIC_QT ON)
    set(CMAKE_STATIC_PREFIX "${INVADER_MINGW_PREFIX}/static")
    set(CMAKE_FIND_ROOT_PATH "${CMAKE_STATIC_PREFIX};${CMAKE_FIND_ROOT_PATH}")

    # Invader should obviously not build with shared libs
    set(BUILD_SHARED_LIBS OFF)

    # We are pretty sure this never works as intended because ".a" also matches ".dll.a", however the AUR Qt static package explicitly checks for this being set.
    set(CMAKE_FIND_LIBRARY_SUFFIXES_OVERRIDE ".a;.lib")

    # Set these
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++ -lwinpthread")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFLAC__NO_DLL")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFLAC__NO_DLL")

    # From https://github.com/Martchus/PKGBUILDs/blob/master/cmake/mingw-w64-static/toolchain-mingw-static.cmake

    # prefer static libraries
    set(OPENSSL_USE_STATIC_LIBS ON)
    set(BOOST_USE_STATIC_LIBS ON)

    # force-use the shared Vulkan library because there's no static one
    # note: The library is not used anyways but required for Vulkan to be considered installed and enabled.
    set(Vulkan_LIBRARY "${INVADER_MINGW_PREFIX}/lib/libvulkan.dll.a" CACHE FILEPATH "shared Vulkan IDC library")

    # workaround limitations in upstream pkg-config files and CMake find modules
    set(pkgcfg_lib_libbrotlicommon_brotlicommon "${CMAKE_STATIC_PREFIX}/lib/libbrotlicommon.a" CACHE INTERNAL "static libbrotlicommon")
    set(pkgcfg_lib_libbrotlienc_brotlienc "${CMAKE_STATIC_PREFIX}/lib/libbrotlienc.a;${CMAKE_STATIC_PREFIX}/lib/libbrotlicommon.a" CACHE INTERNAL "static libbrotliend")
    set(pkgcfg_lib_libbrotlidec_brotlidec "${CMAKE_STATIC_PREFIX}/lib/libbrotlidec.a;${CMAKE_STATIC_PREFIX}/lib/libbrotlicommon.a" CACHE INTERNAL "static libbrotlidec")
    set(libbrotlicommon_STATIC_LDFLAGS "${pkgcfg_lib_libbrotlicommon_brotlicommon}" CACHE INTERNAL "static libbrotlicommon")
    set(libbrotlienc_STATIC_LDFLAGS "${pkgcfg_lib_libbrotlienc_brotlienc}" CACHE INTERNAL "static libbrotliend")
    set(libbrotlidec_STATIC_LDFLAGS "${pkgcfg_lib_libbrotlidec_brotlidec}" CACHE INTERNAL "static libbrotlidec")

    # disable use of find modules that don't work for static libraries
    set(CMAKE_DISABLE_FIND_PACKAGE_harfbuzz TRUE)

    # define dependencies of various static libraries as CMake doesn't pull them reliably automatically
    set(OPENSSL_DEPENDENCIES "-lws2_32;-lgdi32;-lcrypt32" CACHE INTERNAL "dependencies of static OpenSSL libraries")
    set(POSTGRESQL_DEPENDENCIES "-lpgcommon;-lpgport;-lintl;-lssl;-lcrypto;-lshell32;-lws2_32;-lsecur32;-liconv" CACHE INTERNAL "dependencies of static PostgreSQL libraries")
    set(MYSQL_DEPENDENCIES "-lzstd;-lshlwapi;-lgdi32;-lws2_32;-lbcrypt;-lcrypt32;-lsecur32;-ladvapi32;-lpthread;-lz;-lm" CACHE INTERNAL "dependencies of static MySQL/MariaDB libraries")
    set(LIBPNG_DEPENDENCIES "-lz" CACHE INTERNAL "dependencies of static libpng")
    set(GLIB2_DEPENDENCIES "-lintl;-lws2_32;-lole32;-lwinmm;-lshlwapi;-lm" CACHE INTERNAL "dependencies of static Glib2")
    set(FREETYPE_DEPENDENCIES "-lbz2;-lharfbuzz;-lfreetype;-lbrotlidec;-lbrotlicommon" CACHE INTERNAL "dependencies of static FreeType2 library")
    set(HARFBUZZ_DEPENDENCIES "-lglib-2.0;${GLIB2_DEPENDENCIES};-lintl;-lm;-lfreetype;-lgraphite2" CACHE INTERNAL "dependencies of static HarfBuzz library")
    set(DBUS1_DEPENDENCIES "-lws2_32;-liphlpapi;-ldbghelp" CACHE INTERNAL "dependencies of static D-Bus1 library")

    # Replace .dll.a with .a
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
    set(ZLIB_LIBRARY_RELEASE            "${INVADER_MINGW_PREFIX}/lib/libz.a")
    set(pkgcfg_lib_PC_HARFBUZZ_harfbuzz "${INVADER_MINGW_PREFIX}/lib/libharfbuzz.a")
    set(pkgcfg_lib_PC_PCRE2_pcre2-16    "${INVADER_MINGW_PREFIX}/lib/libpcre2-16.a")

# Fix any dependencies needed to be fixed
else()
    set(DEP_SQUISH_LIBRARIES squish gomp)
    set(SDL2_LIBRARIES ${SDL2_STATIC_LIBRARIES} -lsetupapi -limm32 -lversion -lwinmm)
    set(TIFF_LIBRARIES ${TIFF_LIBRARIES} jpeg lzma)
    set(LibArchive_LIBRARIES ${LibArchive_LIBRARIES} lzma zstd bz2 iconv crypto crypt32 bcrypt)
    set(FREETYPE_LIBRARIES freetype png brotlidec brotlicommon bz2 harfbuzz graphite2 freetype)
endif()
