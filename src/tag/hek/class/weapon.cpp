/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#include "../compile.hpp"
#include "../../../hek/constants.hpp"

#include "weapon.hpp"

namespace Invader::HEK {
    void compile_weapon_tag(CompiledTag &compiled, const std::byte *data, std::size_t size, WeaponJasonJones jason_jones) {
        BEGIN_COMPILE(Weapon);
        COMPILE_ITEM_DATA
        ADD_DEPENDENCY_ADJUST_SIZES(tag.ready_effect);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.overheated);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.detonation);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.player_melee_damage);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.player_melee_response);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.actor_firing_parameters);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.light_power_on_effect);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.light_power_off_effect);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.first_person_model);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.first_person_animations);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.hud_interface);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.pickup_sound);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.zoom_in_sound);
        ADD_DEPENDENCY_ADJUST_SIZES(tag.zoom_out_sound);
        ADD_REFLEXIVE(tag.predicted_resources);
        ADD_REFLEXIVE_START(tag.magazines) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.reloading_effect);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.chambering_effect);
            ADD_BASIC_DEPENDENCY_REFLEXIVE(reflexive.magazine_objects, equipment);
        } ADD_REFLEXIVE_END
        ADD_REFLEXIVE_START(tag.triggers) {
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.charging_effect);
            ADD_DEPENDENCY_ADJUST_SIZES(reflexive.projectile);
            ADD_REFLEXIVE_START(reflexive.firing_effects) {
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.firing_effect);
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.misfire_effect);
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.empty_effect);
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.firing_damage);
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.misfire_damage);
                ADD_DEPENDENCY_ADJUST_SIZES(reflexive.empty_damage);
            } ADD_REFLEXIVE_END

            reflexive.illumination_recovery_rate = 1.0f / TICK_RATE / reflexive.illumination_recovery_time;
            reflexive.ejection_port_recovery_rate = 1.0f / TICK_RATE / reflexive.ejection_port_recovery_time;

            reflexive.firing_acceleration_rate = 1.0f / TICK_RATE / reflexive.acceleration_time;
            reflexive.firing_deceleration_rate = 1.0f / TICK_RATE / reflexive.deceleration_time;

            reflexive.error_acceleration_rate = 1.0f / TICK_RATE / reflexive.error_acceleration_time;
            reflexive.error_deceleration_rate = 1.0f / TICK_RATE / reflexive.error_deceleration_time;

            // Jason Jones the accuracy of the weapon
            switch(jason_jones) {
                case WEAPON_JASON_JONES_PISTOL_SINGLEPLAYER:
                    reflexive.minimum_error = DEGREES_TO_RADIANS(0.2F);
                    reflexive.error_angle.from = DEGREES_TO_RADIANS(0.2F);
                    reflexive.error_angle.to = DEGREES_TO_RADIANS(0.4F);
                    break;
                case WEAPON_JASON_JONES_PLASMA_RIFLE_SINGLEPLAYER:
                    reflexive.error_angle.from = DEGREES_TO_RADIANS(0.25F);
                    reflexive.error_angle.to = DEGREES_TO_RADIANS(2.5F);
                    break;
                default:
                    break;
            }

        } ADD_REFLEXIVE_END
        tag.object_type = OBJECT_TYPE_WEAPON;
        FINISH_COMPILE
    }
}
