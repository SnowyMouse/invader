/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "../compile.hpp"
#include "glow.hpp"

namespace Invader::HEK {
    void compile_glow_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Glow)
        ADD_DEPENDENCY_ADJUST_SIZES(tag.texture)
        tag.attachment_0 = static_cast<GlowAttachment>(tag.attachment_0 - 1);
        tag.attachment_1 = static_cast<GlowAttachment>(tag.attachment_1 - 1);
        tag.attachment_2 = static_cast<GlowAttachment>(tag.attachment_2 - 1);
        tag.attachment_3 = static_cast<GlowAttachment>(tag.attachment_3 - 1);
        tag.attachment_4 = static_cast<GlowAttachment>(tag.attachment_4 - 1);
        tag.attachment_5 = static_cast<GlowAttachment>(tag.attachment_5 - 1);
        FINISH_COMPILE
    }
}
