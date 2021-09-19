// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__BUILD__BUILD_HPP
#define INVADER__BUILD__BUILD_HPP

#include <invader/hek/map.hpp>

namespace Invader::Build {
    [[maybe_unused]] static inline std::string get_comma_separated_game_engine_shorthands() {
        std::string engines;
        
        for(auto &i : Invader::Parser::GameEngineInfo::get_all_shorthands()) {
            if(!engines.empty()) {
                engines += ", ";
            }
            engines += i;
        }
        
        return engines;
    }
}

#endif
