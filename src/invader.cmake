# SPDX-License-Identifier: GPL-3.0-only

# Parser files
set(INVADER_PARSER_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/all.hpp"
    
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/actor_variant.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/actor.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/antenna.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/biped.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/bitfield.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/bitmap.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/camera_track.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/color_table.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/continuous_damage_effect.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/contrail.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/damage_effect.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/decal.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/detail_object_collection.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/device_control.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/device_light_fixture.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/device_machine.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/device.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/dialogue.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/effect.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/enum.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/equipment.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/flag.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/fog.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/font.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/garbage.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/gbxmodel.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/globals.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/glow.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/grenade_hud_interface.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/hud_globals.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/hud_interface_types.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/hud_message_text.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/hud_number.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/input_device_defaults.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/item_collection.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/item.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/lens_flare.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/light_volume.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/light.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/lightning.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/material_effects.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/meter.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/model_animations.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/model_collision_geometry.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/model.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/multiplayer_scenario_description.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/object.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/particle_system.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/particle.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/physics.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/placeholder.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/point_physics.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/preferences_network_game.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/projectile.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/scenario_structure_bsp.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/scenario.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/scenery.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/shader_environment.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/shader_model.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/shader_transparent_chicago_extended.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/shader_transparent_chicago.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/shader_transparent_generic.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/shader_transparent_glass.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/shader_transparent_meter.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/shader_transparent_plasma.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/shader_transparent_water.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/shader.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/sky.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/sound_environment.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/sound_looping.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/sound_scenery.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/sound.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/string_list.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/tag_collection.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/ui_widget_definition.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/unicode_string_list.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/unit_hud_interface.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/unit.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/vehicle.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/virtual_keyboard.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/weapon_hud_interface.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/weapon.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/weather_particle_system.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition/wind.hpp"
    
    "${CMAKE_CURRENT_BINARY_DIR}/actor_variant.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/actor.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/antenna.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/biped.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/bitfield.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/bitmap.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/camera_track.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/color_table.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/continuous_damage_effect.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/contrail.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/damage_effect.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/decal.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/detail_object_collection.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/device_control.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/device_light_fixture.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/device_machine.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/device.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/dialogue.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/effect.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/enum.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/equipment.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/flag.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/fog.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/font.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/garbage.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/gbxmodel.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/globals.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/glow.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/grenade_hud_interface.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/hud_globals.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/hud_interface_types.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/hud_message_text.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/hud_number.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/input_device_defaults.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/item_collection.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/item.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/lens_flare.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/light_volume.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/light.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/lightning.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/material_effects.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/meter.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/model_animations.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/model_collision_geometry.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/model.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/multiplayer_scenario_description.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/object.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/particle_system.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/particle.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/physics.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/placeholder.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/point_physics.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/preferences_network_game.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/projectile.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/scenario_structure_bsp.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/scenario.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/scenery.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/shader_environment.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/shader_model.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/shader_transparent_chicago_extended.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/shader_transparent_chicago.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/shader_transparent_generic.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/shader_transparent_glass.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/shader_transparent_meter.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/shader_transparent_plasma.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/shader_transparent_water.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/shader.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/sky.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/sound_environment.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/sound_looping.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/sound_scenery.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/sound.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/string_list.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/tag_collection.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/ui_widget_definition.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/unicode_string_list.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/unit_hud_interface.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/unit.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/vehicle.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/virtual_keyboard.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/weapon_hud_interface.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/weapon.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/weather_particle_system.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/wind.cpp"
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
    "${CMAKE_CURRENT_BINARY_DIR}/mcc-cea-getter.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/resource-list.cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/color_code.cpp"

    ${INVADER_PARSER_FILES}
    ${INVADER_AUDIO_FILES}

    src/error.cpp
    src/hek/fourcc.cpp
    src/hek/data_type.cpp
    src/hek/map.cpp
    src/resource/resource_map.cpp
    src/dependency/found_tag_dependency.cpp
    src/map/map.cpp
    src/map/tag.cpp
    src/file/file.cpp
    src/build/build_workload.cpp
    src/build/build_workload_dedupe.cpp
    src/bitmap/swizzle.cpp
    src/bitmap/bitmap_encode.cpp
    src/bitmap/color_plate_scanner.cpp
    src/bitmap/bitmap_processor.cpp
    src/bitmap/sprite.cpp
    src/error_handler/error_handler.cpp
    src/model/jms.cpp
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
    src/tag/parser/compile/bitmap/compile.cpp
    src/tag/parser/compile/bitmap/decompile.cpp
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
    do_windows_rc(invader libinvader.dll "Invader library")
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

# Include color code script
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/color_code.cpp"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/src/bitmap/color_code/color_codes_generator.py" "${CMAKE_CURRENT_SOURCE_DIR}/src/bitmap/color_code/color_codes" "${CMAKE_CURRENT_BINARY_DIR}/color_code.cpp"
)

# Include definition script
add_custom_command(
    OUTPUT ${INVADER_PARSER_FILES}
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/code_generator" "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/hek/definition" "${CMAKE_CURRENT_SOURCE_DIR}/include/invader/tag/parser/definition" "${CMAKE_CURRENT_BINARY_DIR}"
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
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/mcc-cea-getter.cpp"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/index/generate_index_getter.py" "${CMAKE_CURRENT_BINARY_DIR}/mcc-cea-getter.cpp" "mcc_cea" "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/index/mcc_cea/*"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/tag/index/generate_index_getter.py"
)

# Include version script
if(${IN_GIT_REPO})
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/version_str.hpp"
        COMMAND "${CMAKE_COMMAND}" "-DGIT_EXECUTABLE=${GIT_EXECUTABLE}" "-DGIT_DIR=${CMAKE_CURRENT_SOURCE_DIR}/.git" "-DOUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/version_str.hpp" -DPROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} -DPROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR} -DPROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH} -DPROJECT_NAME=${PROJECT_NAME} -DIN_GIT_REPO=${IN_GIT_REPO} -P ${CMAKE_CURRENT_SOURCE_DIR}/src/version.cmake
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/.git/refs/heads/${INVADER_GIT_BRANCH}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/version.cmake"
    )
else()
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/version_str.hpp"
        COMMAND "${CMAKE_COMMAND}" "-DGIT_EXECUTABLE=${GIT_EXECUTABLE}" "-DGIT_DIR=${CMAKE_CURRENT_SOURCE_DIR}/.git" "-DOUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/version_str.hpp" -DPROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} -DPROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR} -DPROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH} -DPROJECT_NAME=${PROJECT_NAME} -DIN_GIT_REPO=${IN_GIT_REPO} -P ${CMAKE_CURRENT_SOURCE_DIR}/src/version.cmake
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/version.cmake"
    )
endif()

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

# Remove warnings from this
set_source_files_properties(src/bitmap/stb/stb_impl.c PROPERTIES COMPILE_FLAGS -Wno-unused-function)

# Include that
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${ZLIB_INCLUDE_DIRS})

# Link against everything
target_link_libraries(invader invader-bitmap-p8-palette ${CMAKE_THREAD_LIBS_INIT} ${ZLIB_LIBRARIES} ${DEP_AUDIO_LIBRARIES} ${SQUISH_LIBRARIES})
