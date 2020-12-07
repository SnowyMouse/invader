# SPDX-License-Identifier: GPL-3.0-only

# Parser files
set(INVADER_PARSER_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/hek/definition.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/parser.hpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-save-hek-data.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-read-hek-data.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-read-cache-file-data.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-cache-format.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-cache-deformat.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-refactor-reference.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-struct-value.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-check-broken-enums.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-check-invalid-references.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-check-invalid-ranges.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-check-invalid-indices.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-compare.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-normalize.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-read-hek-file.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-check-uppercase-references.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/bitfield.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/enum.cpp"
)

# Sound stuff
if(${INVADER_USE_AUDIO})
    SET(INVADER_AUDIO_FILES
        src/sound/sound_encoder_flac.cpp
        src/sound/sound_encoder_ogg_vorbis.cpp
        src/sound/sound_encoder_wav.cpp
        src/sound/sound_encoder_xbox_adpcm.cpp
        src/sound/sound_encoder.cpp
        src/sound/sound_reader_16_bit_pcm_big_endian.cpp
        src/sound/sound_reader_flac.cpp
        src/sound/sound_reader_ogg.cpp
        src/sound/sound_reader_wav.cpp
        src/sound/sound_reader_xbox_adpcm.cpp
        src/sound/adpcm_xq/adpcm-lib.c
    )
else()
    SET(INVADER_AUDIO_FILES "")
endif()

# Invader library
set(INVADER_SOURCE_FILES
    "${CMAKE_CURRENT_BINARY_DIR}/language.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/retail-getter.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/demo-getter.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/custom-edition-getter.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/resource-list.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/color_code.cpp"

    "${CMAKE_CURRENT_BINARY_DIR}/parser-save-hek-data.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-read-hek-data.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-read-cache-file-data.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-cache-format.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-cache-deformat.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-refactor-reference.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-struct-value.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-check-invalid-references.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-check-invalid-ranges.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-check-invalid-indices.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-check-broken-enums.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-compare.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-normalize.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-read-hek-file.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-check-uppercase-references.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/bitfield.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/enum.cpp"

    ${INVADER_AUDIO_FILES}

    src/hek/class_int.cpp
    src/hek/data_type.cpp
    src/hek/map.cpp
    src/resource/resource_map.cpp
    src/dependency/found_tag_dependency.cpp
    src/map/map.cpp
    src/map/tag.cpp
    src/file/file.cpp
    src/build/build_workload.cpp
    src/bitmap/s3tc/s3tc.cpp
    src/bitmap/swizzle.cpp
    src/bitmap/bitmap_encode.cpp
    src/error_handler/error_handler.cpp
    src/script/compiler.cpp
    src/script/script_tree.cpp
    src/script/tokenizer.cpp
    src/compress/compression.cpp
    src/tag/hek/header.cpp
    src/tag/hek/class/bitmap.cpp
    src/tag/hek/class/model_collision_geometry/intersection_check.cpp
    src/tag/hek/class/model_collision_geometry/model_collision_geometry.cpp
    src/extract/extraction.cpp
    src/tag/parser/parser_struct.cpp
    src/tag/parser/post_cache_deformat.cpp
    src/tag/parser/compile/actor.cpp
    src/tag/parser/compile/antenna.cpp
    src/tag/parser/compile/bitmap.cpp
    src/tag/parser/compile/contrail.cpp
    src/tag/parser/compile/damage_effect.cpp
    src/tag/parser/compile/decal.cpp
    src/tag/parser/compile/detail_object_collection.cpp
    src/tag/parser/compile/effect.cpp
    src/tag/parser/compile/font.cpp
    src/tag/parser/compile/globals.cpp
    src/tag/parser/compile/glow.cpp
    src/tag/parser/compile/hud_interface.cpp
    src/tag/parser/compile/item_collection.cpp
    src/tag/parser/compile/lens_flare.cpp
    src/tag/parser/compile/light.cpp
    src/tag/parser/compile/lightning.cpp
    src/tag/parser/compile/meter.cpp
    src/tag/parser/compile/model.cpp
    src/tag/parser/compile/model_animations.cpp
    src/tag/parser/compile/model_collision_geometry.cpp
    src/tag/parser/compile/object.cpp
    src/tag/parser/compile/particle.cpp
    src/tag/parser/compile/point_physics.cpp
    src/tag/parser/compile/physics.cpp
    src/tag/parser/compile/scenario/decompile.cpp
    src/tag/parser/compile/scenario/post_compile.cpp
    src/tag/parser/compile/scenario/postprocess.cpp
    src/tag/parser/compile/scenario/pre_compile.cpp
    src/tag/parser/compile/scenario_structure_bsp.cpp
    src/tag/parser/compile/shader.cpp
    src/tag/parser/compile/sound.cpp
    src/tag/parser/compile/string_list.cpp

    src/crc/crc32.c
    src/crc/crc_spoof.c
    src/crc/hek/crc.cpp

    src/version.cpp
)

if(NOT DEFINED ${INVADER_STATIC_BUILD})
    set(INVADER_STATIC_BUILD false CACHE BOOL "Create a static build of libinvader.a (NOTE: Does not remove external dependencies)")
endif()

# If static build, build as static
if(${INVADER_STATIC_BUILD})
    add_library(invader STATIC
        ${INVADER_SOURCE_FILES}
    )
# Otherwise, do this
else()
    add_library(invader SHARED
        ${INVADER_SOURCE_FILES}
    )
endif()

# Set our alternative Invader library (the one we aren't linking)
set_target_properties(${ALTERNATE_INVADER_BUILD}
    PROPERTIES OUTPUT_NAME invader
)
set(TARGETS_LIST ${TARGETS_LIST} ${ALTERNATE_INVADER_BUILD})

# Do this
add_custom_target(invader-header-version
    SOURCES "${CMAKE_CURRENT_BINARY_DIR}/version_str.hpp"
)
add_custom_target(invader-header-gen
    SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/hek/definition.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/parser.hpp"
)
add_dependencies(invader invader-header-gen invader-header-version)

# P8 palette library (separate for slightly faster building)
add_library(invader-bitmap-p8-palette STATIC
    "${CMAKE_CURRENT_BINARY_DIR}/p8_palette.cpp"
)

# This is just for memes
option(INVADER_FORCE_PORTABLE_PREFERRED_PATHS "Use forward slashes for all preferred paths (does nothing if it's already a forward slash)")
if(${INVADER_FORCE_PORTABLE_PREFERRED_PATHS})
    add_definitions(-DINVADER_FORCE_PORTABLE_PREFERRED_PATHS)
endif()

# This is fun
option(INVADER_EXTRACT_HIDDEN_VALUES "Extract (most) hidden values; used for debugging Invader ONLY - this WILL break tags")

# Include color code script
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/color_code.cpp"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/src/bitmap/color_code/color_codes_generator.py" "${CMAKE_CURRENT_SOURCE_DIR}/src/bitmap/color_code/color_codes" "${CMAKE_CURRENT_BINARY_DIR}/color_code.cpp"
)

# Include definition script
add_custom_command(
    OUTPUT ${INVADER_PARSER_FILES}
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/code_generator" ${INVADER_PARSER_FILES} ${INVADER_EXTRACT_HIDDEN_VALUES} "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/hek/definition/*"
)

# Include version script
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/p8_palette.cpp"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/src/bitmap/p8/palette.py" "${CMAKE_CURRENT_SOURCE_DIR}/src/bitmap/p8/p8_palette" "${CMAKE_CURRENT_BINARY_DIR}/p8_palette.cpp"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/bitmap/p8/palette.py"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/bitmap/p8/p8_palette"
)

# Include version getters
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/retail-getter.cpp"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/index/generate_index_getter.py" "${CMAKE_CURRENT_BINARY_DIR}/retail-getter.cpp" "retail" "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/index/retail/*"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/index/generate_index_getter.py"
)
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/demo-getter.cpp"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/index/generate_index_getter.py" "${CMAKE_CURRENT_BINARY_DIR}/demo-getter.cpp" "demo" "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/index/demo/*"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/index/generate_index_getter.py"
)
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/custom-edition-getter.cpp"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/index/generate_index_getter.py" "${CMAKE_CURRENT_BINARY_DIR}/custom-edition-getter.cpp" "custom_edition" "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/index/custom_edition/*"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/index/generate_index_getter.py"
)

# Include version script
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/version_str.hpp"
    COMMAND "${CMAKE_COMMAND}" "-DGIT_EXECUTABLE=${GIT_EXECUTABLE}" "-DGIT_DIR=${CMAKE_CURRENT_SOURCE_DIR}/.git" "-DOUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/version_str.hpp" -DPROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} -DPROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR} -DPROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH} -DPROJECT_NAME=${PROJECT_NAME} -DIN_GIT_REPO=${IN_GIT_REPO} -P ${CMAKE_CURRENT_SOURCE_DIR}/src/version.cmake
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/.git/refs/heads/${INVADER_GIT_BRANCH}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/version.cmake"
)

# Make the language.cpp file
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/language.cpp"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/src/info/language/language.py" "${CMAKE_CURRENT_BINARY_DIR}/language.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/info/language/json/*"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/info/language/language.py"
)

# Build the resource list
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/resource-list.cpp"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/generator.py" "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/bitmaps.tag_indices" "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/sounds.tag_indices" "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/loc.tag_indices" "${CMAKE_CURRENT_BINARY_DIR}/resource-list.cpp"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/generator.py" "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/bitmaps.tag_indices" "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/sounds.tag_indices" "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/loc.tag_indices"
)

# Set a constant if we're extracting hidden values
if(${INVADER_EXTRACT_HIDDEN_VALUES})
    add_definitions(-DINVADER_EXTRACT_HIDDEN_VALUES)
endif()

# Remove warnings from this
set_source_files_properties(src/bitmap/stb/stb_impl.c PROPERTIES COMPILE_FLAGS -Wno-unused-function)

# Include that
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${ZLIB_INCLUDE_DIRS})

# Link against everything
target_link_libraries(invader invader-bitmap-p8-palette ${CMAKE_THREAD_LIBS_INIT} zstd ${DEP_ZLIB_LIBRARIES} ${DEP_AUDIO_LIBRARIES})
