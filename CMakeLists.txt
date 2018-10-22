#
# Invader (c) 2018 Kavawuvi
#
# This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
#

# Require at least version 3.11z
cmake_minimum_required(VERSION 3.11)

# Use C++
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)

# Do stuff depending on the compiler
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wold-style-cast")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -static -static-libgcc -static-libstdc++")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -g -march=native")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif()

# Turn on console messages
option(NO_OUTPUT "don't show any console messages" OFF)
if(NO_OUTPUT)
    add_definitions(-DNO_OUTPUT)
endif()

# Invader library
add_library(invader STATIC
    src/hek/class_int.cpp
    src/hek/data_type.cpp
    src/resource/resource_map.cpp

    src/map/map.cpp
    src/map/tag.cpp

    src/tag/hek/compile.cpp
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
    src/tag/hek/class/shader_transparent_glass.cpp
    src/tag/hek/class/shader_transparent_meter.cpp
    src/tag/hek/class/shader_transparent_plasma.cpp
    src/tag/hek/class/shader_transparent_water.cpp
    src/tag/hek/class/sky.cpp
    src/tag/hek/class/sound.cpp
    src/tag/hek/class/sound_environment.cpp
    src/tag/hek/class/sound_looping.cpp
    src/tag/hek/class/tag_collection.cpp
    src/tag/hek/class/ui_widget_definition.cpp
    src/tag/hek/class/unicode_string_list.cpp
    src/tag/hek/class/unit_hud_interface.cpp
    src/tag/hek/class/vehicle.cpp
    src/tag/hek/class/weapon.cpp
    src/tag/hek/class/weapon_hud_interface.cpp
    src/tag/hek/class/wind.cpp
    src/tag/hek/class/virtual_keyboard.cpp
    src/tag/compiled_tag.cpp

    src/error.cpp
)

# Build executable
add_executable(invader-build
    src/build/build.cpp
    src/build/build_workload.cpp
)
target_link_libraries(invader-build invader)

# Indexer executable
add_executable(invader-indexer
    src/indexer/indexer.cpp
)
target_link_libraries(invader-indexer invader)

# Scenario executable
add_executable(invader-scenario
    src/scenario/scenario.cpp
    src/scenario/salamander_scenario.cpp
)
target_link_libraries(invader-scenario invader)
