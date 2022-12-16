// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__PARSER__COMPILE__MODEL_ANIMATIONS_HPP
#define INVADER__TAG__PARSER__COMPILE__MODEL_ANIMATIONS_HPP

namespace Invader::Parser {
    struct ModelAnimationsAnimation;

    /**
     * Read a bit from a bitfield.
     * @param offset bit offset
     * @param fields pointer to fields
     */
    bool read_bit_from_bitfield(std::size_t offset, std::uint32_t *fields) noexcept;

    /**
     * Calculate the expected uncompressed frame size for the animation in bytes.
     * @param animation animation
     */
    std::size_t expected_uncompressed_frame_size_for_animation(ModelAnimationsAnimation &animation) noexcept;
}

#endif
