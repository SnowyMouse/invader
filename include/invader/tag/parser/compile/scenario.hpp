// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__PARSER__COMPILE__SCENARIO_HPP
#define INVADER__TAG__PARSER__COMPILE__SCENARIO_HPP

#include "../parser.hpp"

#include <filesystem>

namespace Invader::Parser {
    /**
     * Fix scenario script source data being missing by decompiling scripts
     * @param  scenario scenario to fix
     * @param  fix      actually apply changes
     * @return          true if the flag was wrong, false if not
     */
    bool fix_missing_script_source_data(Scenario &scenario, bool fix);
    
    /**
     * Compile scripts for the scenario
     * @param scenario           scenario to compile scripts for
     * @param info               target engine info
     * @param warnings           array to hold warnings
     * @param tags_directories   tags directories in order of precedence
     * @param scripts            optional array of scripts (filename-data pairs). If not set, use source data from the scenario tag
     */
    void compile_scripts(Scenario &scenario, const HEK::GameEngineInfo &info, std::vector<std::string> &warnings, const std::vector<std::filesystem::path> &tags_directories, const std::optional<std::vector<std::pair<std::string, std::vector<std::byte>>>> &scripts = std::nullopt);
}

#endif
