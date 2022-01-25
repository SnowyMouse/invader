// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__PARSER__COMPILE__BITMAP_HPP
#define INVADER__TAG__PARSER__COMPILE__BITMAP_HPP

#include <cstdint>

namespace Invader {
    class BuildWorkload;
}

namespace Invader::Parser {
    /**
     * Set the environment flag for the given bitmap tag
     * @param  workload  workload
     * @param  tag_index tag
     */
    void set_bitmap_data_environment_flag(BuildWorkload &workload, std::size_t tag_index);
}

#endif
