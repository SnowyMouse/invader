# SPDX-License-Identifier: GPL-3.0-only

# Invader library
set(INVADER_SOURCE_FILES
    "${CMAKE_CURRENT_BINARY_DIR}/resource_list.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-save-hek-data.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-read-hek-data.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-read-cache-file-data.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-cache-format.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/parser-cache-deformat.cpp"

    src/hek/class_int.cpp
    src/hek/data_type.cpp
    src/hek/map.cpp
    src/resource/resource_map.cpp
    src/dependency/found_tag_dependency.cpp
    src/map/map.cpp
    src/map/tag.cpp
    src/file/file.cpp
    src/build/build_workload.cpp
    src/build2/build_workload.cpp
    src/bitmap/color_plate_scanner.cpp
    src/bitmap/image_loader.cpp
    src/bitmap/bitmap_data_writer.cpp
    src/compress/compression.cpp
    src/tag/hek/compile.cpp
    src/tag/hek/header.cpp
    src/tag/hek/class/actor.cpp
    src/tag/hek/class/actor_variant.cpp
    src/tag/hek/class/antenna.cpp
    src/tag/hek/class/biped.cpp
    src/tag/hek/class/bitmap.cpp
    src/tag/hek/class/camera_track.cpp
    src/tag/hek/class/color_table.cpp
    src/tag/hek/class/contrail.cpp
    src/tag/hek/class/damage_effect.cpp
    src/tag/hek/class/decal.cpp
    src/tag/hek/class/detail_object_collection.cpp
    src/tag/hek/class/device_control.cpp
    src/tag/hek/class/device_light_fixture.cpp
    src/tag/hek/class/device_machine.cpp
    src/tag/hek/class/dialogue.cpp
    src/tag/hek/class/effect.cpp
    src/tag/hek/class/equipment.cpp
    src/tag/hek/class/flag.cpp
    src/tag/hek/class/fog.cpp
    src/tag/hek/class/font.cpp
    src/tag/hek/class/garbage.cpp
    src/tag/hek/class/gbxmodel.cpp
    src/tag/hek/class/globals.cpp
    src/tag/hek/class/glow.cpp
    src/tag/hek/class/grenade_hud_interface.cpp
    src/tag/hek/class/hud_globals.cpp
    src/tag/hek/class/hud_message_text.cpp
    src/tag/hek/class/hud_number.cpp
    src/tag/hek/class/input_device_defaults.cpp
    src/tag/hek/class/item_collection.cpp
    src/tag/hek/class/lens_flare.cpp
    src/tag/hek/class/light.cpp
    src/tag/hek/class/lightning.cpp
    src/tag/hek/class/light_volume.cpp
    src/tag/hek/class/material_effects.cpp
    src/tag/hek/class/meter.cpp
    src/tag/hek/class/model_animations.cpp
    src/tag/hek/class/model_collision_geometry.cpp
    src/tag/hek/class/multiplayer_scenario_description.cpp
    src/tag/hek/class/object.cpp
    src/tag/hek/class/particle.cpp
    src/tag/hek/class/particle_system.cpp
    src/tag/hek/class/point_physics.cpp
    src/tag/hek/class/physics.cpp
    src/tag/hek/class/projectile.cpp
    src/tag/hek/class/scenario.cpp
    src/tag/hek/class/scenario_structure_bsp.cpp
    src/tag/hek/class/shader_environment.cpp
    src/tag/hek/class/shader_model.cpp
    src/tag/hek/class/shader_transparent_chicago.cpp
    src/tag/hek/class/shader_transparent_chicago_extended.cpp
    src/tag/hek/class/shader_transparent_generic.cpp
    src/tag/hek/class/shader_transparent_glass.cpp
    src/tag/hek/class/shader_transparent_meter.cpp
    src/tag/hek/class/shader_transparent_plasma.cpp
    src/tag/hek/class/shader_transparent_water.cpp
    src/tag/hek/class/sky.cpp
    src/tag/hek/class/sound.cpp
    src/tag/hek/class/sound_environment.cpp
    src/tag/hek/class/sound_looping.cpp
    src/tag/hek/class/string_list.cpp
    src/tag/hek/class/tag_collection.cpp
    src/tag/hek/class/ui_widget_definition.cpp
    src/tag/hek/class/unit_hud_interface.cpp
    src/tag/hek/class/vehicle.cpp
    src/tag/hek/class/weapon.cpp
    src/tag/hek/class/weapon_hud_interface.cpp
    src/tag/hek/class/weather_particle_system.cpp
    src/tag/hek/class/wind.cpp
    src/tag/hek/class/virtual_keyboard.cpp
    src/tag/compiled_tag.cpp
    src/extract/extraction.cpp
    src/tag/parser/post_cache_deformat.cpp
    src/tag/parser/compile/bitmap.cpp
    src/tag/parser/compile/object.cpp
    src/tag/parser/compile/shader.cpp
    src/bitmap/stb/stb_impl.c

    src/crc/crc32.c
    src/crc/crc_spoof.c
    src/crc/hek/crc.cpp

    src/version.cpp
)

if(NOT DEFINED ${INVADER_STATIC_BUILD})
    set(INVADER_STATIC_BUILD false CACHE BOOL "Create a static build of libinvader.a (NOTE: Does not remove external dependencies)")
endif()

if(${INVADER_STATIC_BUILD})
    add_library(invader STATIC
        ${INVADER_SOURCE_FILES}
    )
else()
    add_library(invader SHARED
        ${INVADER_SOURCE_FILES}
    )
endif()

# Generate headers separately (this is to guarantee build order)
add_custom_target(invader-header-gen
    SOURCES "${CMAKE_CURRENT_BINARY_DIR}/version_str.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/hek/definition.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/parser.hpp"
)
add_dependencies(invader invader-header-gen)

# P8 palette library (separate for slightly faster building)
add_library(invader-bitmap-p8-palette STATIC
    "${CMAKE_CURRENT_BINARY_DIR}/p8_palette.cpp"
)

# This is fun
option(INVADER_EXTRACT_HIDDEN_VALUES "Have invader-extract extract hidden values (deprecated)")

# Include definition script
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/hek/definition.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/parser.hpp" "${CMAKE_CURRENT_BINARY_DIR}/parser-save-hek-data.cpp" "${CMAKE_CURRENT_BINARY_DIR}/parser-read-hek-data.cpp" "${CMAKE_CURRENT_BINARY_DIR}/parser-read-cache-file-data.cpp" "${CMAKE_CURRENT_BINARY_DIR}/parser-cache-format.cpp" "${CMAKE_CURRENT_BINARY_DIR}/parser-cache-deformat.cpp"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/header_generator.py" "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/hek/definition.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/parser.hpp" "${CMAKE_CURRENT_BINARY_DIR}/parser-save-hek-data.cpp" "${CMAKE_CURRENT_BINARY_DIR}/parser-read-hek-data.cpp" "${CMAKE_CURRENT_BINARY_DIR}/parser-read-cache-file-data.cpp" "${CMAKE_CURRENT_BINARY_DIR}/parser-cache-format.cpp"  "${CMAKE_CURRENT_BINARY_DIR}/parser-cache-deformat.cpp" ${INVADER_EXTRACT_HIDDEN_VALUES} "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/hek/definition/*"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/header_generator.py"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/hek/definition/*"
)

# Include version script
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/p8_palette.cpp"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/src/bitmap/p8/palette.py" "${CMAKE_CURRENT_SOURCE_DIR}/src/bitmap/p8/p8_palette" "${CMAKE_CURRENT_BINARY_DIR}/p8_palette.cpp"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/bitmap/p8/palette.py"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/bitmap/p8/p8_palette"
)

# Include version script
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/version_str.hpp"
    COMMAND "${CMAKE_COMMAND}" "-DGIT_EXECUTABLE=${GIT_EXECUTABLE}" "-DGIT_DIR=${CMAKE_CURRENT_SOURCE_DIR}/.git" "-DOUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/version_str.hpp" -DPROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} -DPROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR} -DPROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH} -DIN_GIT_REPO=${IN_GIT_REPO} -P ${CMAKE_CURRENT_SOURCE_DIR}/src/version.cmake
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/.git/refs/heads/${INVADER_GIT_BRANCH}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/version.cmake"
)

# Build the resource list
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/resource_list.cpp"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/generator.py" "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/bitmaps.tag_indices" "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/sounds.tag_indices" "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/loc.tag_indices" "${CMAKE_CURRENT_BINARY_DIR}/resource_list.cpp"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/generator.py" "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/bitmaps.tag_indices" "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/sounds.tag_indices" "${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/loc.tag_indices"
)

# Here's to set up constants for version.cpp
set_source_files_properties(src/version.cpp
    PROPERTIES COMPILE_DEFINITIONS "INVADER_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} INVADER_VERSION_MINOR=${PROJECT_VERSION_MINOR} INVADER_VERSION_PATCH=${PROJECT_VERSION_PATCH} INVADER_FORK=\"${PROJECT_NAME}\""
)

# Set a constant if we're extracting hidden values
if(${INVADER_EXTRACT_HIDDEN_VALUES})
    set_source_files_properties(src/tag/hek/header.cpp
        PROPERTIES COMPILE_DEFINITIONS "INVADER_EXTRACT_HIDDEN_VALUES"
    )
endif()

# Remove warnings from this
set_source_files_properties(src/bitmap/stb/stb_impl.c PROPERTIES COMPILE_FLAGS -Wno-unused-function)

# Include that
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${TIFF_INCLUDE_DIRS})

# Add libraries
target_link_libraries(invader invader-bitmap-p8-palette zstd ${TIFF_LIBRARIES})
