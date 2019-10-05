// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__ITEM_HPP
#define INVADER__TAG__HEK__CLASS__ITEM_HPP

#include "object.hpp"

namespace Invader::HEK {
    enum ItemFunctionIn : TagEnum {
        ITEM_FUNCTION_IN_NONE,
        ITEM_FUNCTION_IN_BODY_VITALITY,
        ITEM_FUNCTION_IN_SHIELD_VITALITY,
        ITEM_FUNCTION_IN_RECENT_BODY_DAMAGE,
        ITEM_FUNCTION_IN_RECENT_SHIELD_DAMAGE,
        ITEM_FUNCTION_IN_RANDOM_CONSTANT,
        ITEM_FUNCTION_IN_UMBRELLA_SHIELD_VITALITY,
        ITEM_FUNCTION_IN_SHIELD_STUN,
        ITEM_FUNCTION_IN_RECENT_UMBRELLA_SHIELD_VITALITY,
        ITEM_FUNCTION_IN_UMBRELLA_SHIELD_STUN,
        ITEM_FUNCTION_IN_REGION,
        ITEM_FUNCTION_IN_REGION_1,
        ITEM_FUNCTION_IN_REGION_2,
        ITEM_FUNCTION_IN_REGION_3,
        ITEM_FUNCTION_IN_REGION_4,
        ITEM_FUNCTION_IN_REGION_5,
        ITEM_FUNCTION_IN_REGION_6,
        ITEM_FUNCTION_IN_REGION_7,
        ITEM_FUNCTION_IN_ALIVE,
        ITEM_FUNCTION_IN_COMPASS
    };

    struct ItemFlags {
        std::uint32_t always_maintains_z_up : 1;
        std::uint32_t destroyed_by_explosions : 1;
        std::uint32_t unaffected_by_gravity : 1;
    };

    ENDIAN_TEMPLATE(EndianType) struct Item : Object<EndianType> {
        EndianType<ItemFlags> item_flags;
        EndianType<std::int16_t> message_index;
        EndianType<std::int16_t> sort_order;
        EndianType<float> scale;
        EndianType<std::int16_t> hud_message_value_scale;
        PAD(0x2);
        PAD(0x10);
        EndianType<std::int16_t> item_a_in;
        EndianType<std::int16_t> item_b_in;
        EndianType<std::int16_t> item_c_in;
        EndianType<std::int16_t> item_d_in;
        PAD(0xA4);
        TagDependency<EndianType> material_effects; // material_effects
        TagDependency<EndianType> collision_sound; // sound
        PAD(0x78);
        Bounds<EndianType<float>> detonation_delay;
        TagDependency<EndianType> detonating_effect; // effect
        TagDependency<EndianType> detonation_effect; // effect
    };
    static_assert(sizeof(Item<BigEndian>) == 0x308);

    #define COPY_ITEM_DATA COPY_OBJECT_DATA\
                           COPY_THIS(item_flags);\
                           COPY_THIS(message_index);\
                           COPY_THIS(sort_order);\
                           COPY_THIS(scale);\
                           COPY_THIS(hud_message_value_scale);\
                           COPY_THIS(item_a_in);\
                           COPY_THIS(item_b_in);\
                           COPY_THIS(item_c_in);\
                           COPY_THIS(item_d_in);\
                           COPY_THIS(material_effects);\
                           COPY_THIS(collision_sound);\
                           COPY_THIS(detonation_delay);\
                           COPY_THIS(detonating_effect);\
                           COPY_THIS(detonation_effect);

    #define COMPILE_ITEM_DATA COMPILE_OBJECT_DATA\
                              ADD_DEPENDENCY_ADJUST_SIZES(tag.material_effects);\
                              ADD_DEPENDENCY_ADJUST_SIZES(tag.collision_sound);\
                              ADD_DEPENDENCY_ADJUST_SIZES(tag.detonating_effect);\
                              ADD_DEPENDENCY_ADJUST_SIZES(tag.detonation_effect);
}
#endif
