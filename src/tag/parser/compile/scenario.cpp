// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build2/build_workload.hpp>

namespace Invader::Parser {
    void Scenario::pre_compile(BuildWorkload2 &workload, std::size_t tag_index, std::size_t, std::size_t) {
        if(!workload.cache_file_type.has_value()) {
            workload.cache_file_type = this->type;
        }
        else {
            workload.report_error(BuildWorkload2::ErrorType::ERROR_TYPE_FATAL_ERROR, "Multiple scenario tags are used", tag_index);
            throw InvalidTagDataException();
        }

        if(this->scripts.size() > 0) {
            if(this->source_files.size() == 0) {
                eprintf_warn("Scenario tag has script data but no source file data");
                eprintf_warn("This is DEPRECATED and will not be allowed in some future version of Invader.");
                workload.report_error(BuildWorkload2::ErrorType::ERROR_TYPE_WARNING, "To fix this, recompile the scripts", tag_index);
            }
            else {
                // TODO: Recompile scripts
                workload.report_error(BuildWorkload2::ErrorType::ERROR_TYPE_WARNING, "TODO: Implement script re-compiling", tag_index);
            }

            // TODO: Implement reference array rebuilding
            workload.report_error(BuildWorkload2::ErrorType::ERROR_TYPE_WARNING, "TODO: Implement reference array rebuilding", tag_index);
            std::terminate();
        }

        // TODO: Swap endianness for script node table, adding a script syntax data and script string data block if none is present
        std::terminate();
    }

    void Scenario::post_compile(BuildWorkload2 &workload, std::size_t tag_index, std::size_t, std::size_t) {
        // TODO: Position encounters and command lists
        if(this->encounters.size() == 0 || this->command_lists.size() == 0) {
            workload.report_error(BuildWorkload2::ErrorType::ERROR_TYPE_FATAL_ERROR, "TODO: Implement encounter and command list BSP location", tag_index);
            std::terminate();
        }
    }

    void ScenarioCutsceneTitle::pre_compile(BuildWorkload2 &, std::size_t, std::size_t, std::size_t) {
        this->fade_in_time *= TICK_RATE;
        this->fade_out_time *= TICK_RATE;
        this->up_time *= TICK_RATE;
    }
}
