// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EXTRACT__EXTRACTION_HPP
#define INVADER__EXTRACT__EXTRACTION_HPP

#include <vector>
#include "../map/tag.hpp"
#include "../error_handler/error_handler.hpp"

namespace Invader::Parser {
    struct ParserStruct;
}

namespace Invader {
    class ExtractionWorkload : public ErrorHandler {
    public:
        /**
         * Extract a single tag from a map
         * @param tag             tag from a loaded map to extract
         * @param reporting_level reporting level to use
         * @return                extracted tag
         */
        static std::vector<std::byte> extract_single_tag(const Tag &tag, ReportingLevel reporting_level = ReportingLevel::REPORTING_LEVEL_ALL);
        
        /**
         * @param map             map to read
         * @param tags            tags directory to extract to
         * @param queries         search queries to look for
         * @param queries_exclude search queries to exclude
         * @param recursive       also extract tags depended by a tag
         * @param overwrite       overwrite tag files that exist
         * @param non_mp_globals  allow extraction of non-multiplayer globals
         * @param reporting_level reporting level to use
         */
        static void extract_map(const Map &map, const std::string &tags, const std::vector<std::string> &queries, const std::vector<std::string> &queries_exclude, bool recursive = false, bool overwrite = false, bool non_mp_globals = false, ReportingLevel reporting_level = ReportingLevel::REPORTING_LEVEL_ALL);
        
    private:
        /**
         * Extract a tag from the map
         * @param tag tag to extract
         */
        std::optional<std::unique_ptr<Parser::ParserStruct>> extract_tag(std::size_t tag_index);
        
        /**
         * Perform the extraction
         * @param queries         queries to do
         * @param queries_exclude queries to not do
         * @param tags            tag files to output to
         * @param recursive       also extract tags depended by a tag
         * @param overwrite       overwrite tag files that exist
         * @param non_mp_globals  allow extraction of non-multiplayer globals
         * @return                number of tags successfully extracted
         */
        std::size_t perform_extraction(const std::vector<std::string> &queries, const std::vector<std::string> &queries_exclude, const std::filesystem::path &tags, bool recursive, bool overwrite, bool non_mp_globals);
        
        /** Map reference */
        const Map &map;
        
        /** Tags directory */
        std::filesystem::path tags;
        
        /** All tags that were matched */
        std::vector<std::size_t> matched_tags;
        
        ExtractionWorkload(const Map &map, ReportingLevel reporting_level);
        ~ExtractionWorkload() override = default;
    };
}

#endif
