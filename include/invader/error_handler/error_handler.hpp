// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__ERROR_HANDLER__ERROR_HANDLER_HPP
#define INVADER__ERROR_HANDLER__ERROR_HANDLER_HPP

#include <cstddef>
#include <optional>
#include <vector>

#include "../file/file.hpp"

namespace Invader {
    class ErrorHandler {
    public:
        /**
         * Types of errors to use
         */
        enum ErrorType {
            /**
             * These are warnings that can be turned off. They report issues that don't affect the game, but the user may want to be made aware of them.
             */
            ERROR_TYPE_WARNING_PEDANTIC,

            /**
             * These are warnings that cannot be turned off. They report issues that do affect the game but still result in a valid cache file.
             */
            ERROR_TYPE_WARNING,

            /**
             * These are errors that prevent saving the cache file. They report issues that result in a potentially unplayable yet still technically valid cache file.
             */
            ERROR_TYPE_ERROR,

            /**
             * These are errors that halt building the cache file. They report issues that result in invalid and/or unusable cache files or don't result in any cache file at all.
             */
            ERROR_TYPE_FATAL_ERROR
        };
        
        /**
         * Verbosity of the handler
         */
        enum ReportingLevel {
            /**
             * Hide all warnings and errors
             */
            REPORTING_LEVEL_HIDE_EVERYTHING,
            
            /**
             * Hide all warnings
             */
            REPORTING_LEVEL_HIDE_ALL_WARNINGS,
            
            /**
             * Hide only pedantic warnings
             */
            REPORTING_LEVEL_HIDE_ALL_PEDANTIC_WARNINGS,
            
            /**
             * Show all warnings
             */
            REPORTING_LEVEL_ALL
        };
        
        /**
         * Report an error
         * @param type      error type
         * @param error     error message
         * @param tag_index tag index
         */
        void report_error(ErrorType type, const char *error, std::optional<std::size_t> tag_index = std::nullopt);
        
        /**
         * Get the number of warnings reported
         * @return number of warnings
         */
        std::size_t get_warnings() const noexcept {
            return this->warnings;
        }
        
        /**
         * Get the number of errors reported
         * @return number of errors
         */
        std::size_t get_errors() const noexcept {
            return this->errors;
        }
        
        /**
         * Get the reporting level
         * @return reporting level
         */
        ReportingLevel get_reporting_level() const noexcept {
            return this->reporting_level;
        }
        
        /**
         * Set reporting level
         * @param reporting_level reporting level
         */
        void set_reporting_level(ReportingLevel reporting_level) noexcept {
            this->reporting_level = reporting_level;
        }
        
        virtual ~ErrorHandler() = 0;
        
    protected:
        /**
         * Instantiate an error handler
         * @param reporting level to use
         */
        ErrorHandler(ReportingLevel reporting_level = ReportingLevel::REPORTING_LEVEL_ALL) noexcept;
        
        /**
         * Get the tag paths array for error reporting
         */
        std::vector<File::TagFilePath> &get_tag_paths() noexcept {
            return this->tag_paths;
        }
        
    private:
        std::size_t warnings = 0;
        std::size_t errors = 0;
        ReportingLevel reporting_level = ReportingLevel::REPORTING_LEVEL_ALL;
        std::vector<File::TagFilePath> tag_paths;
        
    };
    
    #define REPORT_ERROR_PRINTF(handler, type, tag_index, ...) { \
        char report_error_message[2048]; \
        std::snprintf(report_error_message, sizeof(report_error_message), __VA_ARGS__); \
        (handler).report_error(Invader::ErrorHandler::ErrorType::type, report_error_message, tag_index); \
    }
}

#endif
