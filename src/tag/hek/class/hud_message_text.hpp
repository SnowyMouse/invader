/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__HUD_MESSAGE_TEXT_HPP
#define INVADER__TAG__HEK__CLASS__HUD_MESSAGE_TEXT_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct HUDMessageTextElement {
        std::int8_t type;
        std::int8_t data;

        ENDIAN_TEMPLATE(NewType) operator HUDMessageTextElement<NewType>() const noexcept {
            HUDMessageTextElement<NewType> copy = {};
            COPY_THIS(type);
            COPY_THIS(data);
            return copy;
        }
    };
    static_assert(sizeof(HUDMessageTextElement<BigEndian>) == 0x2);

    ENDIAN_TEMPLATE(EndianType) struct HUDMessageTextMessage {
        TagString name;
        EndianType<std::int16_t> start_index_into_text_blob;
        EndianType<std::int16_t> start_index_of_message_block;
        std::int8_t panel_count;
        PAD(0x3);
        PAD(0x18);

        ENDIAN_TEMPLATE(NewType) operator HUDMessageTextMessage<NewType>() const noexcept {
            HUDMessageTextMessage<NewType> copy = {};
            COPY_THIS(name);
            COPY_THIS(start_index_into_text_blob);
            COPY_THIS(start_index_of_message_block);
            COPY_THIS(panel_count);
            return copy;
        }
    };
    static_assert(sizeof(HUDMessageTextMessage<BigEndian>) == 0x40);

    ENDIAN_TEMPLATE(EndianType) struct HUDMessageText {
        TagDataOffset<EndianType> text_data;
        TagReflexive<EndianType, HUDMessageTextElement> message_elements;
        TagReflexive<EndianType, HUDMessageTextMessage> messages;
        PAD(0x54);

        ENDIAN_TEMPLATE(NewType) operator HUDMessageText<NewType>() const noexcept {
            HUDMessageText<NewType> copy = {};
            COPY_THIS(text_data);
            COPY_THIS(message_elements);
            COPY_THIS(messages);
            return copy;
        }
    };
    static_assert(sizeof(HUDMessageText<BigEndian>) == 0x80);

    void compile_hud_message_text_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
