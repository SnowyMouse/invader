// SPDX-License-Identifier: GPL-3.0-only

#include <invader/build/build_workload.hpp>
#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void EventHandlerReference::post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        // Warn if we have a scenario script set but the script does not exist

        if(!workload.disable_recursion && (this->flags & HEK::EventHandlerReferencesFlagsFlag::EVENT_HANDLER_REFERENCES_FLAGS_FLAG_RUN_SCENARIO_SCRIPT)) {
            auto &scenario_tag = workload.tags[workload.scenario_index];
            auto &scenario_struct = workload.structs[*scenario_tag.base_struct];
            auto &scenario_base = *reinterpret_cast<Parser::Scenario::struct_little *>(scenario_struct.data.data());

            auto scripts_count = static_cast<std::size_t>(scenario_base.scripts.count);
            bool found = false;
            if(scripts_count > 0) {
                auto scripts_offset = scenario_struct.resolve_pointer(&scenario_base.scripts.pointer);
                auto &scripts_struct = workload.structs[*scripts_offset];
                auto *scripts = reinterpret_cast<Parser::ScenarioScript::struct_little *>(scripts_struct.data.data());
                for(std::size_t i = 0; i < scripts_count; i++) {
                    auto &script = scripts[i];
                    if((found = std::strcmp(script.name.string, this->script.string) == 0)) {
                        break;
                    }
                }
            }

            if(!found) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "No script \"%s\" in %s.scenario", this->script.string, File::halo_path_to_preferred_path(scenario_tag.path).c_str());
            }
        }
    }
}
