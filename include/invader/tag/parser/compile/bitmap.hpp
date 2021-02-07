// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__PARSER__COMPILE__BITMAP_HPP
#define INVADER__TAG__PARSER__COMPILE__BITMAP_HPP

#include "../parser.hpp"

namespace Invader::Parser {
    /**
     * Downgrade an invader bitmap tag to a regular bitmap tag
     * @param  tag tag to downgrade
     * @return     downgraded bitmap
     */
    Bitmap downgrade_invader_bitmap(const InvaderBitmap &tag);
}

#endif
