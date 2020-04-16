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
         * @param tag tag from a loaded map to extract
         * @return    extracted tag
         */
        static std::vector<std::byte> extract_tag(const Tag &tag);
    private:
        ExtractionWorkload() = default;
        ~ExtractionWorkload() override = default;
    };
}

#endif
