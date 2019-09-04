# Invader library
add_library(invader STATIC
    src/hek/class_int.cpp
    src/hek/data_type.cpp
    src/resource/resource_map.cpp
    src/dependency/found_tag_dependency.cpp
    src/map/map.cpp
    src/map/tag.cpp
    src/build/build_workload.cpp
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

    src/crc/crc32.c
    src/crc/crc_spoof.c
    src/crc/hek/crc.cpp

    src/error.cpp
    "${CMAKE_CURRENT_BINARY_DIR}/version_str.hpp"
    "${CMAKE_CURRENT_BINARY_DIR}/resource_list.cpp"
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

# Include that
include_directories(${CMAKE_CURRENT_BINARY_DIR})
