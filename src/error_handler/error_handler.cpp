#include <invader/error_handler/error_handler.hpp>
#include <invader/printf.hpp>
#include <invader/error.hpp>

#ifdef __linux__
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_TERMINAL_WIDTH_SUPPORTED 2048

namespace Invader {
    void ErrorHandler::report_error(ErrorType type, const char *error, std::optional<std::size_t> tag_index) {
        // Print the right column (description)
        std::size_t terminal_width = 80;
        
        // Resize based on console width
        #ifdef __linux__
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);   
        std::size_t new_width = static_cast<std::size_t>(w.ws_col);
        if(terminal_width < new_width) {
            terminal_width = new_width;
        }
        #endif
        
        #ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO w;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &w);
        std::size_t new_width = w.srWindow.Right - w.srWindow.Left + 1;
        if(terminal_width < new_width) {
            terminal_width = new_width;
        }
        #endif
        
        // Set a max width
        if(terminal_width > MAX_TERMINAL_WIDTH_SUPPORTED) {
            terminal_width = MAX_TERMINAL_WIDTH_SUPPORTED;
        }
        
        #define WRITE_ERROR_MESSAGE_WRAPPED(call, fmt, ...) { \
            char text_to_output[MAX_TERMINAL_WIDTH_SUPPORTED + 2]; \
            std::size_t offset = 0; \
            std::size_t len = std::snprintf(text_to_output, sizeof(text_to_output), fmt, __VA_ARGS__); \
            if(len > sizeof(text_to_output) - 1) { \
                len = sizeof(text_to_output) - 1; \
            } \
            while(offset < len) { \
                char line[MAX_TERMINAL_WIDTH_SUPPORTED + 2]; \
                std::size_t line_offset = 0; \
                line_offset = 0; \
                line[0] = 0; \
                while(true) { \
                    /* skip spaces */ \
                    for(; offset < len && text_to_output[offset] == ' '; offset++); \
                    std::size_t word_offset = offset; \
                     \
                    /* did we hit a null terminator? */ \
                    if(text_to_output[offset] == 0) { \
                        break; \
                    } \
                     \
                    /* find the end of the word */ \
                    std::size_t word_end = offset; \
                    for(; offset < len && text_to_output[word_end] != ' ' && text_to_output[word_end] != 0; word_end++); \
                     \
                    /* get the number of characters */ \
                    std::size_t word_len = (word_end - word_offset) + 1; \
                     \
                    /* if we have characters in the line, and we're at the end of the line, empty the line */ \
                    if(word_len + line_offset > terminal_width && line_offset > 0) { \
                        call("%s", line); \
                        line_offset = 0; \
                        line[0] = 0; \
                    } \
                     \
                    /* if the length is longer than the terminal width, just print it */ \
                    if(word_len > terminal_width) { \
                        std::snprintf(line, sizeof(line) > word_len ? word_len : sizeof(line), "%s", text_to_output + offset); \
                        call("%s", line); \
                        line[0] = 0; \
                        line_offset = 0; \
                    } \
                    /* otherwise, append it! */ \
                    else { \
                        std::snprintf(line + line_offset, word_len + 1, "%s%s", (line_offset > 0 ? " " : ""), text_to_output + offset); \
                        line_offset += (word_len - 1) + (line_offset > 0 ? 1 : 0); \
                    } \
                    /* advance number of characters */ \
                    offset = word_end; \
                } \
                /* if we have something in the line, print it */ \
                if(line_offset > 0) { \
                    call("%s", line); \
                } \
            } \
        }
        
        switch(type) {
            case ErrorType::ERROR_TYPE_WARNING_PEDANTIC:
                if(this->reporting_level <= ReportingLevel::REPORTING_LEVEL_HIDE_ALL_PEDANTIC_WARNINGS) {
                    return;
                }
                WRITE_ERROR_MESSAGE_WRAPPED(eprintf_warn_lesser, "WARNING (minor): %s", error);
                this->warnings++;
                break;
            case ErrorType::ERROR_TYPE_WARNING:
                if(this->reporting_level <= ReportingLevel::REPORTING_LEVEL_HIDE_ALL_WARNINGS) {
                    return;
                }
                WRITE_ERROR_MESSAGE_WRAPPED(eprintf_warn, "WARNING: %s", error);
                this->warnings++;
                break;
            case ErrorType::ERROR_TYPE_ERROR:
                if(this->reporting_level <= ReportingLevel::REPORTING_LEVEL_HIDE_EVERYTHING) {
                    return;
                }
                WRITE_ERROR_MESSAGE_WRAPPED(eprintf_error, "ERROR: %s", error);
                this->errors++;
                break;
            case ErrorType::ERROR_TYPE_FATAL_ERROR:
                if(this->reporting_level <= ReportingLevel::REPORTING_LEVEL_HIDE_EVERYTHING) {
                    return;
                }
                WRITE_ERROR_MESSAGE_WRAPPED(eprintf_error, "FATAL ERROR: %s", error);
                this->errors++;
                break;
        }
        
        if(tag_index.has_value()) {
            auto index = *tag_index;
            auto tag_count = this->tag_paths.size();
            if(index >= tag_count) {
                eprintf_error("Tried to report an error for an invalid tag (%zu >= %zu)", index, tag_count);
                std::terminate();
            }
            auto &tag = this->tag_paths[index];
            eprintf("...in %s.%s\n", File::halo_path_to_preferred_path(tag.path).c_str(), tag_class_to_extension(tag.class_int));
        }
    }
    
    ErrorHandler::ErrorHandler(ReportingLevel reporting_level) noexcept : reporting_level(reporting_level) {}
    
    ErrorHandler::~ErrorHandler() {}
}
