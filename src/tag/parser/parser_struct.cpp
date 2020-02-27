// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser_struct.hpp>

namespace Invader::Parser {
    ParserStructValue::NumberFormat ParserStructValue::get_number_format() const noexcept {
        if(this->type < ValueType::VALUE_TYPE_FLOAT) {
            return NumberFormat::NUMBER_FORMAT_INT;
        }
        else if(type < ValueType::VALUE_TYPE_DATA) {
            return NumberFormat::NUMBER_FORMAT_FLOAT;
        }
        else {
            return NumberFormat::NUMBER_FORMAT_NONE;
        }

        switch(this->type) {
            case VALUE_TYPE_INT8:
            case VALUE_TYPE_UINT8:
            case VALUE_TYPE_INT16:
            case VALUE_TYPE_UINT16:
            case VALUE_TYPE_INT32:
            case VALUE_TYPE_UINT32:
            case VALUE_TYPE_COLOR_ARGB8:
                return NumberFormat::NUMBER_FORMAT_INT;

            case VALUE_TYPE_FLOAT:
            case VALUE_TYPE_ANGLE:
            case VALUE_TYPE_COLOR_ARGB:
            case VALUE_TYPE_COLOR_RGB:
            case VALUE_TYPE_VECTOR2D:
            case VALUE_TYPE_VECTOR3D:
            case VALUE_TYPE_PLANE2D:
            case VALUE_TYPE_PLANE3D:
            case VALUE_TYPE_POINT2D:
            case VALUE_TYPE_POINT3D:
                return NumberFormat::NUMBER_FORMAT_FLOAT;

            case VALUE_TYPE_QUATERNION:
            case VALUE_TYPE_DATA:
            case VALUE_TYPE_REFLEXIVE:
            case VALUE_TYPE_DEPENDENCY:
            case VALUE_TYPE_BITMASK:
                return NumberFormat::NUMBER_FORMAT_NONE;
        }
    }

    std::size_t ParserStructValue::get_value_count() const noexcept {
        switch(this->type) {
            case VALUE_TYPE_INT8:
            case VALUE_TYPE_UINT8:
            case VALUE_TYPE_INT16:
            case VALUE_TYPE_UINT16:
            case VALUE_TYPE_INT32:
            case VALUE_TYPE_UINT32:
                return 1;

            case VALUE_TYPE_COLOR_ARGB8:
                return 4;

            case VALUE_TYPE_FLOAT:
            case VALUE_TYPE_ANGLE:
                return 1;

            case VALUE_TYPE_COLOR_ARGB:
                return 4;

            case VALUE_TYPE_COLOR_RGB:
                return 3;

            case VALUE_TYPE_VECTOR2D:
                return 2;

            case VALUE_TYPE_VECTOR3D:
                return 3;

            case VALUE_TYPE_PLANE2D:
                return 3;

            case VALUE_TYPE_PLANE3D:
                return 4;

            case VALUE_TYPE_POINT2D:
                return 2;

            case VALUE_TYPE_POINT3D:
                return 3;

            case VALUE_TYPE_QUATERNION:
                return 4;

            case VALUE_TYPE_DATA:
            case VALUE_TYPE_REFLEXIVE:
            case VALUE_TYPE_DEPENDENCY:
            case VALUE_TYPE_BITMASK:
                return 0;
        }
    }

    void ParserStructValue::get_values(Number *values) const noexcept {
        switch(this->type) {
            case VALUE_TYPE_INT8:
                *values = static_cast<std::int64_t>(*reinterpret_cast<const std::int8_t *>(this->address));
                return;

            case VALUE_TYPE_UINT8:
                *values = static_cast<std::int64_t>(*reinterpret_cast<const std::uint8_t *>(this->address));
                return;

            case VALUE_TYPE_INT16:
                *values = static_cast<std::int64_t>(*reinterpret_cast<const std::int16_t *>(this->address));
                return;

            case VALUE_TYPE_UINT16:
                *values = static_cast<std::int64_t>(*reinterpret_cast<const std::uint16_t *>(this->address));
                return;

            case VALUE_TYPE_INT32:
                *values = static_cast<std::int64_t>(*reinterpret_cast<const std::int32_t *>(this->address));
                return;

            case VALUE_TYPE_UINT32:
                *values = static_cast<std::int64_t>(*reinterpret_cast<const std::uint32_t *>(this->address));
                return;

            case VALUE_TYPE_COLOR_ARGB8: {
                const auto &color = *reinterpret_cast<const HEK::ColorARGBInt *>(this->address);
                values[0] = static_cast<std::int64_t>(color.alpha);
                values[1] = static_cast<std::int64_t>(color.red);
                values[2] = static_cast<std::int64_t>(color.green);
                values[3] = static_cast<std::int64_t>(color.blue);
                return;
            }

            case VALUE_TYPE_FLOAT:
            case VALUE_TYPE_ANGLE:
                *values = static_cast<double>(*reinterpret_cast<const float *>(this->address));
                return;

            case VALUE_TYPE_COLOR_ARGB: {
                const auto &color = *reinterpret_cast<const HEK::ColorARGB<HEK::NativeEndian> *>(this->address);
                values[0] = static_cast<double>(color.alpha);
                values[1] = static_cast<double>(color.red);
                values[2] = static_cast<double>(color.green);
                values[3] = static_cast<double>(color.blue);
                return;
            }

            case VALUE_TYPE_COLOR_RGB: {
                const auto &color = *reinterpret_cast<const HEK::ColorRGB<HEK::NativeEndian> *>(this->address);
                values[0] = static_cast<double>(color.red);
                values[1] = static_cast<double>(color.green);
                values[2] = static_cast<double>(color.blue);
                return;
            }

            case VALUE_TYPE_VECTOR2D: {
                const auto &vector = *reinterpret_cast<const HEK::Vector2D<HEK::NativeEndian> *>(this->address);
                values[0] = static_cast<double>(vector.i);
                values[1] = static_cast<double>(vector.j);
                return;
            }

            case VALUE_TYPE_VECTOR3D: {
                const auto &vector = *reinterpret_cast<const HEK::Vector3D<HEK::NativeEndian> *>(this->address);
                values[0] = static_cast<double>(vector.i);
                values[1] = static_cast<double>(vector.j);
                values[2] = static_cast<double>(vector.k);
                return;
            }

            case VALUE_TYPE_PLANE2D: {
                const auto &plane = *reinterpret_cast<const HEK::Plane2D<HEK::NativeEndian> *>(this->address);
                values[0] = static_cast<double>(plane.vector.i);
                values[1] = static_cast<double>(plane.vector.j);
                values[2] = static_cast<double>(plane.w);
                return;
            }

            case VALUE_TYPE_PLANE3D: {
                const auto &plane = *reinterpret_cast<const HEK::Plane3D<HEK::NativeEndian> *>(this->address);
                values[0] = static_cast<double>(plane.vector.i);
                values[1] = static_cast<double>(plane.vector.j);
                values[2] = static_cast<double>(plane.vector.k);
                values[3] = static_cast<double>(plane.w);
                return;
            }

            case VALUE_TYPE_POINT2D: {
                const auto &point = *reinterpret_cast<const HEK::Point2D<HEK::NativeEndian> *>(this->address);
                values[0] = static_cast<double>(point.x);
                values[1] = static_cast<double>(point.y);
                return;
            }

            case VALUE_TYPE_POINT3D: {
                const auto &point = *reinterpret_cast<const HEK::Point3D<HEK::NativeEndian> *>(this->address);
                values[0] = static_cast<double>(point.x);
                values[1] = static_cast<double>(point.y);
                values[2] = static_cast<double>(point.z);
                return;
            }

            case VALUE_TYPE_QUATERNION: {
                const auto &quaternion = *reinterpret_cast<const HEK::Quaternion<HEK::NativeEndian> *>(this->address);
                values[0] = static_cast<double>(quaternion.i);
                values[1] = static_cast<double>(quaternion.j);
                values[2] = static_cast<double>(quaternion.k);
                values[3] = static_cast<double>(quaternion.w);
                return;
            }

            case VALUE_TYPE_DATA:
            case VALUE_TYPE_REFLEXIVE:
            case VALUE_TYPE_DEPENDENCY:
            case VALUE_TYPE_BITMASK:
                eprintf_error("Tried to use get_values() with a type that doesn't have values");
                std::terminate();
        }
    }

    void ParserStructValue::set_values(const Number *values) const noexcept {
        switch(this->type) {
            case VALUE_TYPE_INT8:
                *reinterpret_cast<std::int8_t *>(this->address) = std::get<std::int64_t>(*values);
                return;

            case VALUE_TYPE_UINT8:
                *reinterpret_cast<std::uint8_t *>(this->address) = std::get<std::int64_t>(*values);
                return;

            case VALUE_TYPE_INT16:
                *reinterpret_cast<std::int16_t *>(this->address) = std::get<std::int64_t>(*values);
                return;

            case VALUE_TYPE_UINT16:
                *reinterpret_cast<std::uint16_t *>(this->address) = std::get<std::int64_t>(*values);
                return;

            case VALUE_TYPE_INT32:
                *reinterpret_cast<std::int32_t *>(this->address) = std::get<std::int64_t>(*values);
                return;

            case VALUE_TYPE_UINT32:
                *reinterpret_cast<std::uint32_t *>(this->address) = std::get<std::int64_t>(*values);
                return;

            case VALUE_TYPE_COLOR_ARGB8: {
                auto &color = *reinterpret_cast<HEK::ColorARGBInt *>(this->address);
                color.alpha = static_cast<std::uint8_t>(std::get<int64_t>(values[0]));
                color.red = static_cast<std::uint8_t>(std::get<int64_t>(values[1]));
                color.green = static_cast<std::uint8_t>(std::get<int64_t>(values[2]));
                color.blue = static_cast<std::uint8_t>(std::get<int64_t>(values[3]));
                return;
            }

            case VALUE_TYPE_FLOAT:
            case VALUE_TYPE_ANGLE:
                *reinterpret_cast<float *>(this->address) = static_cast<float>(std::get<double>(*values));
                return;

            case VALUE_TYPE_COLOR_ARGB: {
                auto &color = *reinterpret_cast<HEK::ColorARGB<HEK::NativeEndian> *>(this->address);
                color.alpha = static_cast<float>(std::get<double>(values[0]));
                color.red = static_cast<float>(std::get<double>(values[1]));
                color.green = static_cast<float>(std::get<double>(values[2]));
                color.blue = static_cast<float>(std::get<double>(values[3]));
                return;
            }

            case VALUE_TYPE_COLOR_RGB: {
                auto &color = *reinterpret_cast<HEK::ColorRGB<HEK::NativeEndian> *>(this->address);
                color.red = static_cast<float>(std::get<double>(values[0]));
                color.green = static_cast<float>(std::get<double>(values[1]));
                color.blue = static_cast<float>(std::get<double>(values[2]));
                return;
            }

            case VALUE_TYPE_VECTOR2D: {
                auto &vector = *reinterpret_cast<HEK::Vector2D<HEK::NativeEndian> *>(this->address);
                vector.i = static_cast<float>(std::get<double>(values[0]));
                vector.j = static_cast<float>(std::get<double>(values[1]));
                return;
            }

            case VALUE_TYPE_VECTOR3D: {
                auto &vector = *reinterpret_cast<HEK::Vector3D<HEK::NativeEndian> *>(this->address);
                vector.i = static_cast<float>(std::get<double>(values[0]));
                vector.j = static_cast<float>(std::get<double>(values[1]));
                vector.k = static_cast<float>(std::get<double>(values[2]));
                return;
            }

            case VALUE_TYPE_PLANE2D: {
                auto &plane = *reinterpret_cast<HEK::Plane2D<HEK::NativeEndian> *>(this->address);
                plane.vector.i = static_cast<float>(std::get<double>(values[0]));
                plane.vector.j = static_cast<float>(std::get<double>(values[1]));
                plane.w = static_cast<float>(std::get<double>(values[2]));
                return;
            }

            case VALUE_TYPE_PLANE3D: {
                auto &plane = *reinterpret_cast<HEK::Plane3D<HEK::NativeEndian> *>(this->address);
                plane.vector.i = static_cast<float>(std::get<double>(values[0]));
                plane.vector.j = static_cast<float>(std::get<double>(values[1]));
                plane.vector.k = static_cast<float>(std::get<double>(values[2]));
                plane.w = static_cast<float>(std::get<double>(values[3]));
                return;
            }

            case VALUE_TYPE_POINT2D: {
                auto &point = *reinterpret_cast<HEK::Point2D<HEK::NativeEndian> *>(this->address);
                point.x = static_cast<float>(std::get<double>(values[0]));
                point.y = static_cast<float>(std::get<double>(values[1]));
                return;
            }

            case VALUE_TYPE_POINT3D: {
                auto &point = *reinterpret_cast<HEK::Point3D<HEK::NativeEndian> *>(this->address);
                point.x = static_cast<float>(std::get<double>(values[0]));
                point.y = static_cast<float>(std::get<double>(values[1]));
                point.z = static_cast<float>(std::get<double>(values[2]));
                return;
            }

            case VALUE_TYPE_QUATERNION: {
                auto &quaternion = *reinterpret_cast<HEK::Quaternion<HEK::NativeEndian> *>(this->address);
                quaternion.i = static_cast<float>(std::get<double>(values[0]));
                quaternion.j = static_cast<float>(std::get<double>(values[1]));
                quaternion.k = static_cast<float>(std::get<double>(values[2]));
                quaternion.w = static_cast<float>(std::get<double>(values[3]));
                return;
            }

            case VALUE_TYPE_DATA:
            case VALUE_TYPE_REFLEXIVE:
            case VALUE_TYPE_DEPENDENCY:
            case VALUE_TYPE_BITMASK:
                eprintf_error("Tried to use set_values() with a type that doesn't have values");
                std::terminate();
        }
    }
}
