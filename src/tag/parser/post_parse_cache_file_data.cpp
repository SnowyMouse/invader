// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void Invader::Parser::ActorVariant::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        this->grenade_velocity *= TICK_RATE;
    }

    void Invader::Parser::Bitmap::post_parse_cache_file_data(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer) {
        eprintf("unimplemented\n");
        throw std::exception();
    }

    void Invader::Parser::Font::post_parse_cache_file_data(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer) {
        eprintf("unimplemented\n");
        throw std::exception();
    }

    void Invader::Parser::GBXModel::post_parse_cache_file_data(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer) {
        for(auto &marker : this->markers) {
            for(auto &instance : marker.instances) {
                // Figure out the region
                std::size_t region_index = instance.region_index;
                std::size_t region_count = this->regions.size();
                if(region_index >= region_count) {
                    eprintf("invalid region index %zu / %zu\n", region_index, region_count);
                    throw OutOfBoundsException();
                }

                // Next, figure out the region permutation
                auto &region = this->regions[region_index];
                std::size_t permutation_count = region.permutations.size();
                std::size_t permutation_index = instance.permutation_index;
                if(permutation_index >= permutation_count) {
                    eprintf("invalid permutation index %zu / %zu for region #%zu\n", permutation_index, permutation_count, region_index);
                    throw OutOfBoundsException();
                }

                // Lastly
                auto &new_marker = region.permutations[permutation_index].markers.emplace_back();
                new_marker.name = marker.name;
                new_marker.node_index = instance.node_index;
                new_marker.rotation = instance.rotation;
                new_marker.translation = instance.translation;
            }
        }
        this->markers.clear();
    }

    void Invader::Parser::GlobalsFallingDamage::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        this->maximum_falling_distance = static_cast<float>((this->maximum_falling_distance * this->maximum_falling_distance) / GRAVITY / 2.0f);
        this->harmful_falling_velocity.from = static_cast<float>((this->harmful_falling_velocity.from * this->harmful_falling_velocity.from) / GRAVITY / 2.0f);
        this->harmful_falling_velocity.to = static_cast<float>((this->harmful_falling_velocity.to * this->harmful_falling_velocity.to) / GRAVITY / 2.0f);
    }

    void Invader::Parser::Glow::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        this->attachment_0 = static_cast<HEK::FunctionOut>(this->attachment_0 + 1);
        this->attachment_1 = static_cast<HEK::FunctionOut>(this->attachment_1 + 1);
        this->attachment_2 = static_cast<HEK::FunctionOut>(this->attachment_2 + 1);
        this->attachment_3 = static_cast<HEK::FunctionOut>(this->attachment_3 + 1);
        this->attachment_4 = static_cast<HEK::FunctionOut>(this->attachment_4 + 1);
        this->attachment_5 = static_cast<HEK::FunctionOut>(this->attachment_5 + 1);
    }

    void Invader::Parser::PointPhysics::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        this->air_friction /= 10000.0f;
        this->water_friction /= 10000.0f;
    }

    void Invader::Parser::Projectile::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        this->initial_velocity *= TICK_RATE;
        this->final_velocity *= TICK_RATE;
    }

    void Invader::Parser::ScenarioCutsceneTitle::post_parse_cache_file_data(const Invader::Tag &, std::optional<HEK::Pointer>) {
        this->fade_in_time *= TICK_RATE;
        this->fade_out_time *= TICK_RATE;
        this->up_time *= TICK_RATE;
    }

    void Invader::Parser::ScenarioStructureBSP::post_parse_cache_file_data(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer) {
        eprintf("unimplemented\n");
        throw std::exception();
    }

    void Invader::Parser::ScenarioStructureBSPMaterial::post_parse_cache_file_data(const Invader::Tag &tag, std::optional<HEK::Pointer> pointer) {
        eprintf("unimplemented\n");
        throw std::exception();
    }
}
