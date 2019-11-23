// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EXTRACT__EXTRACTION_HPP
#define INVADER__EXTRACT__EXTRACTION_HPP

#include <vector>
#include "../map/tag.hpp"

namespace Invader::Extraction {
    /**
     * Extract the tag into an HEK tag file
     * @param tag tag to extract
     */
    std::vector<std::byte> extract_tag(const Tag &tag);
}

#endif
