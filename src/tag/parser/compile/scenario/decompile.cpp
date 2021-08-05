// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/definition/scenario.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/hek/class/model_collision_geometry.hpp>
#include <invader/script/compiler.hpp>
#include <invader/tag/parser/compile/scenario.hpp>

namespace Invader::Parser {
    bool fix_missing_script_source_data(Scenario &scenario, bool fix) {
        if(scenario.source_files.size() != 0 || (scenario.scripts.size() == 0 && scenario.globals.size() == 0)) {
            return false;
        }
        
        if(fix) {
            try {
                auto script = Tokenizer::detokenize(ScriptTree::decompile_script_tree(Compiler::decompile_scenario(scenario)));
                const auto *script_data = reinterpret_cast<const std::byte *>(script.c_str());
                auto &source_file = scenario.source_files.emplace_back();
                std::snprintf(source_file.name.string, sizeof(source_file.name.string), "extracted");
                source_file.source = std::vector<std::byte>(script_data, script_data + script.size());
            }
            catch(std::exception &e) {
                eprintf_error("Failed to decompile scripts; scenario will not have any source data: %s", e.what());
                scenario.source_files.clear();
                return false;
            }
        }
        return true;
    }

    void Scenario::post_cache_deformat() {
        auto *script_data = this->script_syntax_data.data();
        auto script_data_size = this->script_syntax_data.size();

        // If we don't have a script node table, give up
        if(script_data_size < sizeof(ScenarioScriptNodeTable::C<LittleEndian>)) {
            eprintf_error("scenario tag has an invalid scenario script node table");
            throw InvalidTagDataException();
        }

        // Copy the table header
        ScenarioScriptNodeTable::C<BigEndian> table = *reinterpret_cast<ScenarioScriptNodeTable::C<LittleEndian> *>(script_data);
        *reinterpret_cast<ScenarioScriptNodeTable::C<BigEndian> *>(script_data) = table;

        // Make sure it's not bullshit
        auto *script_nodes = reinterpret_cast<ScenarioScriptNode::C<LittleEndian> *>(script_data + sizeof(table));
        auto table_size = table.maximum_count.read();
        std::size_t expected_size = (reinterpret_cast<std::byte *>(script_nodes + table_size) - script_data);
        if(expected_size != script_data_size) {
            eprintf_error("scenario tag has an invalid scenario script node table (%zu vs %zu)", expected_size, script_data_size);
            throw InvalidTagDataException();
        }

        // Copy the rest of the table
        for(std::size_t i = 0; i < table_size; i++) {
            ScenarioScriptNode::C<BigEndian> big = script_nodes[i];
            *reinterpret_cast<ScenarioScriptNode::C<BigEndian> *>(script_nodes + i) = big;
        }

        // Put scripts in if need be
        fix_missing_script_source_data(*this, true);

        // And lastly, for consistency sake, remove all tag IDs and zero out the pointer
        this->postprocess_hek_data();
    }
}
