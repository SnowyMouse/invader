/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__EQUIPMENT_HPP
#define INVADER__TAG__HEK__CLASS__EQUIPMENT_HPP

#include "item.hpp"

namespace Invader::HEK {
    enum EquipmentPowerupType : TagEnum {
        EQUIPMENT_POWERUP_TYPE_NONE,
        EQUIPMENT_POWERUP_TYPE_DOUBLE_SPEED,
        EQUIPMENT_POWERUP_TYPE_OVER_SHIELD,
        EQUIPMENT_POWERUP_TYPE_ACTIVE_CAMOUFLAGE,
        EQUIPMENT_POWERUP_TYPE_FULL_SPECTRUM_VISION,
        EQUIPMENT_POWERUP_TYPE_HEALTH,
        EQUIPMENT_POWERUP_TYPE_GRENADE
    };

    enum EquipmentGrenadeType : TagEnum  {
        EQUIPMENT_GRENADE_TYPE_HUMAN_FRAGMENTATION,
        EQUIPMENT_GRENADE_TYPE_COVENANT_PLASMA
    };

    ENDIAN_TEMPLATE(EndianType) struct Equipment : Item<EndianType> {
        EndianType<EquipmentPowerupType> powerup_type;
        EndianType<EquipmentGrenadeType> grenade_type;
        EndianType<float> powerup_time;
        TagDependency<EndianType> pickup_sound; // sound

        PAD(0x90);

        ENDIAN_TEMPLATE(NewType) operator Equipment<NewType>() const noexcept {
            Equipment<NewType> copy = {};
            COPY_ITEM_DATA
            COPY_THIS(powerup_type);
            COPY_THIS(grenade_type);
            COPY_THIS(powerup_time);
            COPY_THIS(pickup_sound);
            return copy;
        }
    };
    static_assert(sizeof(Equipment<BigEndian>) == 0x3B0);

    void compile_equipment_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
