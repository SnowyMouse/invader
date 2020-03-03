// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include "tag_editor_widget.hpp"
#include "tag_editor_window.hpp"
#include "tag_editor_edit_widget.hpp"

namespace Invader::EditQt {
    TagEditorWidget::TagEditorWidget(QWidget *parent, Parser::ParserStructValue *struct_value, TagEditorWindow *editor_window) : QWidget(parent), struct_value(struct_value), editor_window(editor_window) {}

    TagEditorWidget *TagEditorWidget::generate_widget(QWidget *parent, Parser::ParserStructValue *struct_value, TagEditorWindow *editor_window) {
        switch(struct_value->get_type()) {
            case Parser::ParserStructValue::VALUE_TYPE_INT8:
            case Parser::ParserStructValue::VALUE_TYPE_UINT8:
            case Parser::ParserStructValue::VALUE_TYPE_INT16:
            case Parser::ParserStructValue::VALUE_TYPE_UINT16:
            case Parser::ParserStructValue::VALUE_TYPE_INDEX:
            case Parser::ParserStructValue::VALUE_TYPE_INT32:
            case Parser::ParserStructValue::VALUE_TYPE_UINT32:
            case Parser::ParserStructValue::VALUE_TYPE_FLOAT:
            case Parser::ParserStructValue::VALUE_TYPE_FRACTION:
            case Parser::ParserStructValue::VALUE_TYPE_ANGLE:
            case Parser::ParserStructValue::VALUE_TYPE_COLORARGBINT:
            case Parser::ParserStructValue::VALUE_TYPE_POINT2DINT:
            case Parser::ParserStructValue::VALUE_TYPE_RECTANGLE2D:
            case Parser::ParserStructValue::VALUE_TYPE_COLORARGB:
            case Parser::ParserStructValue::VALUE_TYPE_COLORRGB:
            case Parser::ParserStructValue::VALUE_TYPE_VECTOR2D:
            case Parser::ParserStructValue::VALUE_TYPE_VECTOR3D:
            case Parser::ParserStructValue::VALUE_TYPE_EULER2D:
            case Parser::ParserStructValue::VALUE_TYPE_EULER3D:
            case Parser::ParserStructValue::VALUE_TYPE_PLANE2D:
            case Parser::ParserStructValue::VALUE_TYPE_PLANE3D:
            case Parser::ParserStructValue::VALUE_TYPE_POINT2D:
            case Parser::ParserStructValue::VALUE_TYPE_POINT3D:
            case Parser::ParserStructValue::VALUE_TYPE_QUATERNION:
            case Parser::ParserStructValue::VALUE_TYPE_MATRIX:
            case Parser::ParserStructValue::VALUE_TYPE_TAGSTRING:
            case Parser::ParserStructValue::VALUE_TYPE_TAGDATAOFFSET:
            case Parser::ParserStructValue::VALUE_TYPE_ENUM:
            case Parser::ParserStructValue::VALUE_TYPE_BITMASK:
            case Parser::ParserStructValue::VALUE_TYPE_DEPENDENCY:
                return new TagEditorEditWidget(parent, struct_value, editor_window);

            case Parser::ParserStructValue::VALUE_TYPE_REFLEXIVE:
                break;
        }

        return nullptr;
    }

    void TagEditorWidget::value_changed() {
        this->get_editor_window()->make_dirty(true);
    }
}
