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
    
    /**
     * Fix the power-of-two flags on a bitmap tag
     * @param  tag tag to fix
     * @param  fix actually apply changes
     * @return     true if the flag was wrong, false if not
     */
    bool fix_power_of_two(Bitmap &tag, bool fix);
    
    /**
     * Fix the power-of-two flags on a bitmap tag
     * @param  tag tag to fix
     * @param  fix actually apply changes
     * @return     true if the flag was wrong, false if not
     */
    bool fix_power_of_two(InvaderBitmap &tag, bool fix);
    
    /**
     * Fix the power-of-two flags on a bitmap data
     * @param  data data to fix
     * @param  fix  actually apply changes
     * @return      true if the flag was wrong, false if not
     */
    bool fix_power_of_two(BitmapData &data, bool fix);
}

#endif
