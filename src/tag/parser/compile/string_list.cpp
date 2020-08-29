// SPDX-License-Identifier: GPL-3.0-only

#include <invader/build/build_workload.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/tag/parser/compile/string_list.hpp>

namespace Invader::Parser {
    enum StringError {
        STRING_ERROR_OK = 0,
        STRING_ERROR_EMPTY,
        STRING_ERROR_INVALID_SIZE,
        STRING_ERROR_MISSING_NULL_TERMINATOR,
        STRING_ERROR_EXTRA_NULL_TERMINATOR,
        STRING_ERROR_IMPROPER_LINE_ENDINGS
    };
    
    template <typename CHAR, typename T> static StringError check_string(T &what, bool fix) {
        auto size = what.string.size();
        auto *start = reinterpret_cast<const CHAR *>(what.string.data());
        auto *end = reinterpret_cast<const CHAR *>(what.string.data() + size);
        static constexpr const std::size_t CHAR_SIZE = sizeof(CHAR);
        
        // Do we have an empty string? If so, there's a problem.
        if(size == 0) {
            if(fix) {
                // Add a null terminator
                what.string.insert(what.string.end(), CHAR_SIZE, static_cast<std::byte>(0));
            }
            return StringError::STRING_ERROR_EMPTY;
        }
        
        // Do we have a string that's missing part of a character? If so, there's a problem.
        else if(size % sizeof(CHAR) != 0) {
            if(fix) {
                // Add a bytes (string is PROBABLY completely broken but whatever)
                what.string.insert(what.string.end(), CHAR_SIZE - (size % CHAR_SIZE), static_cast<std::byte>(0));
            }
            return StringError::STRING_ERROR_INVALID_SIZE;
        }
        
        // Is the string not null terminated? That's pretty bad.
        else if(end[-1] != 0) {
            if(fix) {
                // Add a null terminator
                what.string.insert(what.string.end(), CHAR_SIZE, static_cast<std::byte>(0));
            }
            return StringError::STRING_ERROR_MISSING_NULL_TERMINATOR;
        }
        
        // Now we need to scan the string
        else {
            // First, check to see if it has an EXTRA null byte in it (it shouldn't! that's bad)
            auto *c = start;
            while(*c) {
                c++;
            }
            
            // And it's broken. Oops!
            if(c + 1 != end) {
                // Looks like we need to rebuild the string with a proper terminator
                if(fix) {
                    std::vector<CHAR> new_vector;
                    new_vector.reserve(size / CHAR_SIZE);
                    
                    // Basically, add every character that isn't a null terminator
                    c = start;
                    while(c < end) {
                        new_vector.push_back(*c);
                        c++;
                    }
                    new_vector.push_back(0); // null terminator of course
                    
                    // Clear the string and then set it to this
                    what.string.clear();
                    what.string.insert(what.string.end(), reinterpret_cast<std::byte *>(new_vector.data()), reinterpret_cast<std::byte *>(new_vector.data() + new_vector.size()));
                }
                return StringError::STRING_ERROR_EXTRA_NULL_TERMINATOR;
            }
            
            // Now, check to see if it has invalid line endings
            c = start;
            bool improper_line_endings = false;
            while(*c) {
                // \n without a preceeding \r
                if(*c == '\n' && (c == start || c[-1] != '\r')) {
                    improper_line_endings = true;
                    break;
                }
                // \r without a succeeding \n
                else if(*c == '\r' && c[1] != '\n') {
                    improper_line_endings = true;
                    break;
                }
                c++;
            }
            if(improper_line_endings) {
                // We need to rebuild the string?
                if(fix) {
                    std::vector<CHAR> new_vector;
                    new_vector.reserve(size / sizeof(CHAR) * 2);
                    
                    // Basically, add every character that isn't a null terminator
                    c = start;
                    while(*c) {
                        // \n without a preceeding \r
                        if(*c == '\n' && (c == start || c[-1] != '\r')) {
                            new_vector.emplace_back('\r');
                            new_vector.emplace_back('\n');
                        }
                        // \r without a succeeding \n
                        else if(*c == '\r' && c[1] != '\n') {
                            new_vector.emplace_back('\r');
                            new_vector.emplace_back('\n');
                        }
                        // It's fine
                        else {
                            new_vector.emplace_back(*c);
                        }
                        c++;
                    }
                    
                    new_vector.push_back(0); // null terminator of course
                    
                    // Clear the string and then set it to this
                    what.string.clear();
                    what.string.insert(what.string.end(), reinterpret_cast<std::byte *>(new_vector.data()), reinterpret_cast<std::byte *>(new_vector.data() + new_vector.size()));
                }
                return StringError::STRING_ERROR_IMPROPER_LINE_ENDINGS;
            }
        }
        
        // No error
        return StringError::STRING_ERROR_OK;
    }
    
    template <typename CHAR, typename T> static bool fix_string_list(T &list, bool fix) {
        std::size_t string_count = list.strings.size();
        bool error_found = false;
        for(std::size_t i = 0; i < string_count; i++) {
            auto &string = list.strings[i];
            bool error_found_in_this_string = check_string<CHAR>(string, fix);
            if(!fix && error_found_in_this_string) {
                return true;
            }
            error_found = error_found || error_found_in_this_string;
        }
        return error_found;
    }
    
    bool fix_broken_strings(StringList &list, bool fix) {
        return fix_string_list<char>(list, fix);
    }
    
    bool fix_broken_strings(UnicodeStringList &list, bool fix) {
        return fix_string_list<char16_t>(list, fix);
    }
    
    template <typename CHAR, typename T> static void string_list_pre_compile(T *what, BuildWorkload &workload, std::size_t tag_index, std::size_t offset) {
        std::size_t index = offset / sizeof(*what);
        
        switch(check_string<CHAR>(*what, false)) {
            case STRING_ERROR_OK:
                return;
            case STRING_ERROR_EMPTY:
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "String #%zu is empty", index);
                break;
            case STRING_ERROR_INVALID_SIZE:
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "String #%zu's size (%zu) is not divisible by its character size (%zu)", index, what->string.size(), sizeof(CHAR));
                break;
            case STRING_ERROR_MISSING_NULL_TERMINATOR:
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "String #%zu is not null terminated", index);
                break;
            case STRING_ERROR_EXTRA_NULL_TERMINATOR:
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "String #%zu contains an extraneous null terminator", index);
                break;
            case STRING_ERROR_IMPROPER_LINE_ENDINGS:
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_ERROR, tag_index, "String #%zu contains a non-CRLF line ending", index);
                break;
        }
    }
    
    void StringListString::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t offset) {
        string_list_pre_compile<char>(this, workload, tag_index, offset);
    }
    void UnicodeStringListString::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t offset) {
        string_list_pre_compile<char16_t>(this, workload, tag_index, offset);
    }
}
