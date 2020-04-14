#include <invader/error_handler/error_handler.hpp>
#include <invader/printf.hpp>
#include <invader/error.hpp>

namespace Invader {
    void ErrorHandler::report_error(ErrorType type, const char *error, std::optional<std::size_t> tag_index) {
        switch(type) {
            case ErrorType::ERROR_TYPE_WARNING_PEDANTIC:
                if(this->reporting_level <= ReportingLevel::REPORTING_LEVEL_HIDE_ALL_PEDANTIC_WARNINGS) {
                    return;
                }
                eprintf_warn_lesser("WARNING (minor): %s", error);
                this->warnings++;
                break;
            case ErrorType::ERROR_TYPE_WARNING:
                if(this->reporting_level <= ReportingLevel::REPORTING_LEVEL_HIDE_ALL_WARNINGS) {
                    return;
                }
                eprintf_warn("WARNING: %s", error);
                this->warnings++;
                break;
            case ErrorType::ERROR_TYPE_ERROR:
                if(this->reporting_level <= ReportingLevel::REPORTING_LEVEL_HIDE_EVERYTHING) {
                    return;
                }
                eprintf_error("ERROR: %s", error);
                this->errors++;
                break;
            case ErrorType::ERROR_TYPE_FATAL_ERROR:
                if(this->reporting_level <= ReportingLevel::REPORTING_LEVEL_HIDE_EVERYTHING) {
                    return;
                }
                eprintf_error("FATAL ERROR: %s", error);
                this->errors++;
                break;
        }
        if(tag_index.has_value()) {
            auto index = *tag_index;
            auto tag_count = this->tag_paths.size();
            if(index >= tag_count) {
                eprintf_error("Tried to report an error for an invalid tag (%zu >= %zu)", index, tag_count);
                throw OutOfBoundsException();
            }
            auto &tag = this->tag_paths[index];
            eprintf("...in %s.%s\n", File::halo_path_to_preferred_path(tag.path).c_str(), tag_class_to_extension(tag.class_int));
        }
    }
    
    ErrorHandler::ErrorHandler(ReportingLevel reporting_level) noexcept : reporting_level(reporting_level) {}
    
    ErrorHandler::~ErrorHandler() {}
}
