// SPDX-License-Identifier: GPL-3.0-only

#include <invader/build/build_workload.hpp>
#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    template <typename CHAR, typename T> static void string_list_pre_compile(const T *what, BuildWorkload &workload, std::size_t tag_index, std::size_t offset) {
        auto size = what->string.size();
        
        // Do we have an empty string? If so, there's a problem.
        if(size == 0) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "String #%zu is empty", offset / sizeof(*what));
        }
        
        // Do we have a string that's missing part of a character? If so, there's a problem.
        else if(size % sizeof(CHAR) != 0) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "String #%zu's size (%zu) is not divisible by its character size (%zu)", offset / sizeof(*what), size, sizeof(CHAR));
        }
        
        // Is the string not null terminated? That's pretty bad.
        else if(*(reinterpret_cast<const CHAR *>(what->string.data() + size) - 1) != 0) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "String #%zu is not null terminated", offset / sizeof(*what));
        }
    }
    
    void StringListString::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t offset) {
        string_list_pre_compile<char>(this, workload, tag_index, offset);
    }
    void UnicodeStringListString::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t offset) {
        string_list_pre_compile<char16_t>(this, workload, tag_index, offset);
    }
}
