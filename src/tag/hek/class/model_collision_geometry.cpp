/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../../../hek/constants.hpp"
#include "../compile.hpp"

#include "model_collision_geometry.hpp"

namespace Invader::HEK {
	void compile_model_collision_geometry_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(ModelCollisionGeometry);

		ADD_DEPENDENCY_ADJUST_SIZES(tag.localized_damage_effect);
		ADD_DEPENDENCY_ADJUST_SIZES(tag.area_damage_effect);
		ADD_DEPENDENCY_ADJUST_SIZES(tag.body_damaged_effect);
		ADD_DEPENDENCY_ADJUST_SIZES(tag.body_depleted_effect);
		ADD_DEPENDENCY_ADJUST_SIZES(tag.body_destroyed_effect);
		ADD_DEPENDENCY_ADJUST_SIZES(tag.shield_damaged_effect);
		ADD_DEPENDENCY_ADJUST_SIZES(tag.shield_depleted_effect);
		ADD_DEPENDENCY_ADJUST_SIZES(tag.shield_recharging_effect);
		tag.shield_recharge_rate = 1.0f / tag.recharge_time / TICK_RATE;

		ADD_REFLEXIVE(tag.materials);
		ADD_REFLEXIVE_START(tag.regions) {
			ADD_DEPENDENCY_ADJUST_SIZES(reflexive.destroyed_effect);
			ADD_REFLEXIVE(reflexive.permutations);
		} ADD_REFLEXIVE_END;
		ADD_REFLEXIVE(tag.modifiers);
		ADD_REFLEXIVE(tag.pathfinding_spheres);
		ADD_REFLEXIVE_START(tag.nodes) {
			ADD_MODEL_COLLISION_BSP(reflexive.bsps)
		} ADD_REFLEXIVE_END

        FINISH_COMPILE
	}
}
