// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void Invader::Parser::ActorVariant::post_parse_cache_file_data(Invader::Tag const &, std::optional<HEK::Pointer>) {
        this->grenade_velocity *= TICK_RATE;
    }

    void Invader::Parser::Bitmap::post_parse_cache_file_data(Invader::Tag const &tag, std::optional<HEK::Pointer> pointer) {
        eprintf("unimplemented\n");
        throw std::exception();
    }

    void Invader::Parser::Font::post_parse_cache_file_data(Invader::Tag const &tag, std::optional<HEK::Pointer> pointer) {
        eprintf("unimplemented\n");
        throw std::exception();
    }

    void Invader::Parser::GBXModel::post_parse_cache_file_data(Invader::Tag const &tag, std::optional<HEK::Pointer> pointer) {
        eprintf("unimplemented\n");
        throw std::exception();
    }

    void Invader::Parser::GlobalsFallingDamage::post_parse_cache_file_data(Invader::Tag const &tag, std::optional<HEK::Pointer> pointer) {
        this->maximum_falling_distance = static_cast<float>((this->maximum_falling_distance * this->maximum_falling_distance) / GRAVITY / 2.0f);
        this->harmful_falling_velocity.from = static_cast<float>((this->harmful_falling_velocity.from * this->harmful_falling_velocity.from) / GRAVITY / 2.0f);
        this->harmful_falling_velocity.to = static_cast<float>((this->harmful_falling_velocity.to * this->harmful_falling_velocity.to) / GRAVITY / 2.0f);
    }

    void Invader::Parser::Glow::post_parse_cache_file_data(Invader::Tag const &tag, std::optional<HEK::Pointer> pointer) {
        this->attachment_0 = static_cast<HEK::FunctionOut>(this->attachment_0 + 1);
        this->attachment_1 = static_cast<HEK::FunctionOut>(this->attachment_1 + 1);
        this->attachment_2 = static_cast<HEK::FunctionOut>(this->attachment_2 + 1);
        this->attachment_3 = static_cast<HEK::FunctionOut>(this->attachment_3 + 1);
        this->attachment_4 = static_cast<HEK::FunctionOut>(this->attachment_4 + 1);
        this->attachment_5 = static_cast<HEK::FunctionOut>(this->attachment_5 + 1);
    }

    void Invader::Parser::PointPhysics::post_parse_cache_file_data(Invader::Tag const &tag, std::optional<HEK::Pointer> pointer) {
        this->air_friction /= 10000.0f;
        this->water_friction /= 10000.0f;
    }

    void Invader::Parser::Projectile::post_parse_cache_file_data(Invader::Tag const &tag, std::optional<HEK::Pointer> pointer) {
        this->initial_velocity *= TICK_RATE;
        this->final_velocity *= TICK_RATE;
    }

    void Invader::Parser::ScenarioCutsceneTitle::post_parse_cache_file_data(Invader::Tag const &tag, std::optional<HEK::Pointer> pointer) {
        this->fade_in_time *= TICK_RATE;
        this->fade_out_time *= TICK_RATE;
        this->up_time *= TICK_RATE;
    }

    void Invader::Parser::ScenarioStructureBSP::post_parse_cache_file_data(Invader::Tag const &tag, std::optional<HEK::Pointer> pointer) {
        eprintf("unimplemented\n");
        throw std::exception();
    }

    void Invader::Parser::ScenarioStructureBSPMaterial::post_parse_cache_file_data(Invader::Tag const &tag, std::optional<HEK::Pointer> pointer) {
        eprintf("unimplemented\n");
        throw std::exception();
    }
}
