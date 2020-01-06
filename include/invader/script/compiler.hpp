// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__SCRIPT__COMPILER_HPP
#define INVADER__SCRIPT__COMPILER_HPP

#include "script_tree.hpp"

namespace Invader::Compiler {
    /**
     * Compile the script tree into the scenario
     * @param script_tree   syntax tree to compile
     * @param scenario      scenario tag to compile into
     * @param error         if an error occurs, this will be set to true
     * @param error_message if an error occurs, this will be the error message
     */
    void compile_script_tree(const std::vector<ScriptTree::Object> &script_tree, Parser::Scenario &scenario, bool &error, std::string &error_message);

    /**
     * Decompile the scenario tag
     * @param  scenario      scenario tag to compile into
     * @param  error         if an error occurs, this will be set to true
     * @param  error_message if an error occurs, this will be the error message
     * @return
     */
    std::vector<ScriptTree::Object> decompile_scenario(const Parser::Scenario &scenario, bool &error, std::string &error_message);
}

#endif
