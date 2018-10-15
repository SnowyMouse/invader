/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#pragma once

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct MaterialEffectsMaterialEffectMaterial {
        TagDependency<EndianType> effect; // effect
        TagDependency<EndianType> sound; // sound
        PAD(0x10);

        ENDIAN_TEMPLATE(NewType) operator MaterialEffectsMaterialEffectMaterial<NewType>() const noexcept {
            MaterialEffectsMaterialEffectMaterial<NewType> copy = {};
            COPY_THIS(effect);
            COPY_THIS(sound);
            return copy;
        }
    };
    static_assert(sizeof(MaterialEffectsMaterialEffectMaterial<BigEndian>) == 0x30);

    ENDIAN_TEMPLATE(EndianType) struct MaterialEffectsMaterialEffect {
        TagReflexive<EndianType, MaterialEffectsMaterialEffectMaterial> materials;
        PAD(0x10);

        ENDIAN_TEMPLATE(NewType) operator MaterialEffectsMaterialEffect<NewType>() const noexcept {
            MaterialEffectsMaterialEffect<NewType> copy = {};
            COPY_THIS(materials);
            return copy;
        }
    };
    static_assert(sizeof(MaterialEffectsMaterialEffect<BigEndian>) == 0x1C);

    ENDIAN_TEMPLATE(EndianType) struct MaterialEffects {
        TagReflexive<EndianType, MaterialEffectsMaterialEffect> effects;
        PAD(0x80);

        ENDIAN_TEMPLATE(NewType) operator MaterialEffects<NewType>() const noexcept {
            MaterialEffects<NewType> copy = {};
            COPY_THIS(effects);
            return copy;
        }
    };
    static_assert(sizeof(MaterialEffects<BigEndian>) == 0x8C);

    void compile_material_effects_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
