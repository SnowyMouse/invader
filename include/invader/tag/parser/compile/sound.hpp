// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__PARSER__COMPILE__SOUND_HPP
#define INVADER__TAG__PARSER__COMPILE__SOUND_HPP

#include "../parser.hpp"

namespace Invader::Parser {
    /**
     * Downgrade an invader sound tag to a regular sound tag
     * @param  tag tag to downgrade
     * @return     downgraded sound
     */
    Sound downgrade_invader_sound(const InvaderSound &tag);
}

#endif
