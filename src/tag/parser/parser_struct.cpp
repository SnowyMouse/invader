// SPDX-License-Identifier: GPL-3.0-only

#include <cassert>
#include <invader/tag/parser/parser.hpp>
#include <invader/tag/parser/parser_struct.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/file/file.hpp>
#include "../../crc/crc32.h"

namespace Invader::Parser {
    ParserStructValue::ParserStructValue(
        const char *       name,
        const char *       member_name,
        const char *       comment,
        Dependency *       dependency,
        const TagFourCC *allowed_classes,
        std::size_t        count,
        bool               read_only
    ) : name(name),
        member_name(member_name),
        comment(comment),
        type(ValueType::VALUE_TYPE_DEPENDENCY),
        address(dependency),
        allowed_classes(allowed_classes, allowed_classes + count),
        read_only(read_only) {}


    ParserStructValue::ParserStructValue(
        const char *name,
        const char *comment
    ) : name(name), comment(comment), type(ValueType::VALUE_TYPE_GROUP_START) {}

    ParserStructValue::ParserStructValue(
        const char *          name,
        const char *          member_name,
        const char *          comment,
        void *                object,
        ValueType             type,
        const char *          unit,
        std::size_t           count,
        bool                  bounds,
        bool                  volatile_value,
        bool                  read_only,
        std::optional<Number> minimum,
        std::optional<Number> maximum
    ) : name(name),
        member_name(member_name),
        comment(comment),
        type(type),
        address(object),
        count(count),
        bounds(bounds),
        unit(unit),
        minimum(minimum),
        maximum(maximum),
        volatile_value(volatile_value),
        read_only(read_only) {}

    ParserStructValue::ParserStructValue(
        const char *                        name,
        const char *                        member_name,
        const char *                        comment,
        void *                              array,
        get_object_in_array_fn_type         get_object_in_array_fn,
        get_array_size_fn_type              get_array_size_fn,
        delete_objects_in_array_fn_type     delete_objects_in_array_fn,
        insert_objects_in_array_fn_type     insert_objects_in_array_fn,
        duplicate_objects_in_array_fn_type  duplicate_objects_in_array_fn,
        swap_objects_in_array_fn_type       swap_objects_in_array_fn,
        std::size_t                         minimum_array_size,
        std::size_t                         maximum_array_size,
        bool                                read_only
    ) : name(name),
        member_name(member_name),
        comment(comment),
        type(ValueType::VALUE_TYPE_REFLEXIVE),
        address(array),
        get_object_in_array_fn(get_object_in_array_fn),
        get_array_size_fn(get_array_size_fn),
        delete_objects_in_array_fn(delete_objects_in_array_fn),
        insert_objects_in_array_fn(insert_objects_in_array_fn),
        duplicate_objects_in_array_fn(duplicate_objects_in_array_fn),
        swap_objects_in_array_fn(swap_objects_in_array_fn),
        min_array_size(minimum_array_size),
        max_array_size(maximum_array_size),
        read_only(read_only) {}

    ParserStructValue::ParserStructValue(
        const char *    name,
        const char *    member_name,
        const char *    comment,
        HEK::TagString *string,
        bool            read_only
    ) : name(name),
        member_name(member_name),
        comment(comment),
        type(ValueType::VALUE_TYPE_TAGSTRING),
        address(string),
        read_only(read_only) {}

    ParserStructValue::ParserStructValue(
        const char *            name,
        const char *            member_name,
        const char *            comment,
        std::vector<std::byte> *offset,
        bool                    read_only
    ) : name(name),
        member_name(member_name),
        comment(comment),
        type(ValueType::VALUE_TYPE_TAGDATAOFFSET),
        address(offset),
        read_only(read_only) {}

    ParserStructValue::ParserStructValue(
        const char *       name,
        const char *       member_name,
        const char *       comment,
        void *             value,
        list_enum_fn_type  list_enum_fn,
        list_enum_fn_type  list_enum_pretty_fn,
        read_enum_fn_type  read_enum_fn,
        write_enum_fn_type write_enum_fn,
        bool               read_only
    ) : name(name),
        member_name(member_name),
        comment(comment),
        type(ValueType::VALUE_TYPE_ENUM),
        address(value),
        list_enum_fn(list_enum_fn),
        list_enum_pretty_fn(list_enum_pretty_fn),
        read_enum_fn(read_enum_fn),
        write_enum_fn(write_enum_fn),
        read_only(read_only) {}

   ParserStructValue::ParserStructValue(
       const char *           name,
       const char *           member_name,
       const char *           comment,
       void *                 value,
       list_enum_fn_type      list_enum_fn,
       list_enum_fn_type      list_enum_pretty_fn,
       read_bitfield_fn_type  read_bitfield_fn,
       write_bitfield_fn_type write_bitfield_fn,
       bool                   read_only
   ) : name(name),
       member_name(member_name),
       comment(comment),
       type(ValueType::VALUE_TYPE_BITMASK),
       address(value),
       list_enum_fn(list_enum_fn),
       list_enum_pretty_fn(list_enum_pretty_fn),
       read_bitfield_fn(read_bitfield_fn),
       write_bitfield_fn(write_bitfield_fn),
       read_only(read_only) {}

    ParserStructValue::NumberFormat ParserStructValue::get_number_format() const noexcept {
        if(this->type < ValueType::VALUE_TYPE_FLOAT) {
            return NumberFormat::NUMBER_FORMAT_INT;
        }
        else if(type < ValueType::VALUE_TYPE_REFLEXIVE) {
            return NumberFormat::NUMBER_FORMAT_FLOAT;
        }
        else {
            return NumberFormat::NUMBER_FORMAT_NONE;
        }
    }

    std::size_t ParserStructValue::get_value_count() const noexcept {
        switch(this->type) {
            case VALUE_TYPE_INT8:
            case VALUE_TYPE_UINT8:
            case VALUE_TYPE_INT16:
            case VALUE_TYPE_INDEX:
            case VALUE_TYPE_UINT16:
            case VALUE_TYPE_INT32:
            case VALUE_TYPE_UINT32:
            case VALUE_TYPE_ENUM:
            case VALUE_TYPE_BITMASK:
                return 1 * this->count;
            case VALUE_TYPE_POINT2DINT:
                return 2 * this->count;
            case VALUE_TYPE_RECTANGLE2D:
            case VALUE_TYPE_COLORARGBINT:
                return 4 * this->count;

            case VALUE_TYPE_MATRIX:
                return 9 * this->count;

            case VALUE_TYPE_FLOAT:
            case VALUE_TYPE_ANGLE:
            case VALUE_TYPE_FRACTION:
                return 1 * this->count;

            case VALUE_TYPE_COLORARGB:
                return 4 * this->count;

            case VALUE_TYPE_COLORRGB:
                return 3 * this->count;

            case VALUE_TYPE_EULER2D:
            case VALUE_TYPE_VECTOR2D:
                return 2 * this->count;

            case VALUE_TYPE_EULER3D:
            case VALUE_TYPE_VECTOR3D:
                return 3 * this->count;

            case VALUE_TYPE_PLANE2D:
                return 3 * this->count;

            case VALUE_TYPE_PLANE3D:
                return 4 * this->count;

            case VALUE_TYPE_POINT2D:
                return 2 * this->count;

            case VALUE_TYPE_POINT3D:
                return 3 * this->count;

            case VALUE_TYPE_QUATERNION:
                return 4 * this->count;

            case VALUE_TYPE_REFLEXIVE:
            case VALUE_TYPE_DEPENDENCY:
            case VALUE_TYPE_TAGID:
            case VALUE_TYPE_TAGSTRING:
            case VALUE_TYPE_TAGDATAOFFSET:
            case VALUE_TYPE_GROUP_START:
                return 0;
        }

        std::terminate();
    }

    std::vector<ParserStructValue::Number> ParserStructValue::get_values() const {
        std::vector<Number> values(this->get_value_count());
        this->get_values(values.data());
        return values;
    }

    void ParserStructValue::get_values(Number *values) const noexcept {
        const auto *addr = reinterpret_cast<const std::byte *>(this->address);
        for(std::size_t i = 0; i < this->count; i++) {
            switch(this->type) {
                case VALUE_TYPE_INT8:
                    *values = static_cast<std::int64_t>(*reinterpret_cast<const std::int8_t *>(addr));
                    addr += sizeof(std::int8_t);
                    values++;
                    break;

                case VALUE_TYPE_UINT8:
                    *values = static_cast<std::int64_t>(*reinterpret_cast<const std::uint8_t *>(addr));
                    addr += sizeof(std::uint8_t);
                    values++;
                    break;

                case VALUE_TYPE_INT16:
                    *values = static_cast<std::int64_t>(*reinterpret_cast<const std::int16_t *>(addr));
                    addr += sizeof(std::int16_t);
                    values++;
                    break;

                case VALUE_TYPE_UINT16:
                case VALUE_TYPE_INDEX:
                case VALUE_TYPE_ENUM:
                case VALUE_TYPE_BITMASK:
                    *values = static_cast<std::int64_t>(*reinterpret_cast<const std::uint16_t *>(addr));
                    addr += sizeof(std::uint16_t);
                    values++;
                    break;

                case VALUE_TYPE_INT32:
                    *values = static_cast<std::int64_t>(*reinterpret_cast<const std::int32_t *>(addr));
                    addr += sizeof(std::int32_t);
                    values++;
                    break;

                case VALUE_TYPE_UINT32:
                    *values = static_cast<std::int64_t>(*reinterpret_cast<const std::uint32_t *>(addr));
                    addr += sizeof(std::uint32_t);
                    values++;
                    break;

                case VALUE_TYPE_COLORARGBINT: {
                    const auto &color = *reinterpret_cast<const HEK::ColorARGBInt *>(addr);
                    values[0] = static_cast<std::int64_t>(color.alpha);
                    values[1] = static_cast<std::int64_t>(color.red);
                    values[2] = static_cast<std::int64_t>(color.green);
                    values[3] = static_cast<std::int64_t>(color.blue);
                    addr += sizeof(color);
                    values += 4;
                    break;
                }

                case VALUE_TYPE_RECTANGLE2D: {
                    const auto &rectangle = *reinterpret_cast<const HEK::Rectangle2D<HEK::NativeEndian> *>(addr);
                    values[0] = static_cast<std::int64_t>(rectangle.top);
                    values[1] = static_cast<std::int64_t>(rectangle.left);
                    values[2] = static_cast<std::int64_t>(rectangle.bottom);
                    values[3] = static_cast<std::int64_t>(rectangle.right);
                    addr += sizeof(rectangle);
                    values += 4;
                    break;
                }

                case VALUE_TYPE_FLOAT:
                case VALUE_TYPE_FRACTION:
                case VALUE_TYPE_ANGLE:
                    *values = static_cast<double>(*reinterpret_cast<const float *>(addr));
                    addr += sizeof(float);
                    values++;
                    break;

                case VALUE_TYPE_COLORARGB: {
                    const auto &color = *reinterpret_cast<const HEK::ColorARGB<HEK::NativeEndian> *>(addr);
                    values[0] = static_cast<double>(color.alpha);
                    values[1] = static_cast<double>(color.red);
                    values[2] = static_cast<double>(color.green);
                    values[3] = static_cast<double>(color.blue);
                    addr += sizeof(color);
                    values += 4;
                    break;
                }

                case VALUE_TYPE_COLORRGB: {
                    const auto &color = *reinterpret_cast<const HEK::ColorRGB<HEK::NativeEndian> *>(addr);
                    values[0] = static_cast<double>(color.red);
                    values[1] = static_cast<double>(color.green);
                    values[2] = static_cast<double>(color.blue);
                    addr += sizeof(color);
                    values += 3;
                    break;
                }

                case VALUE_TYPE_VECTOR2D: {
                    const auto &vector = *reinterpret_cast<const HEK::Vector2D<HEK::NativeEndian> *>(addr);
                    values[0] = static_cast<double>(vector.i);
                    values[1] = static_cast<double>(vector.j);
                    addr += sizeof(vector);
                    values += 2;
                    break;
                }

                case VALUE_TYPE_VECTOR3D: {
                    const auto &vector = *reinterpret_cast<const HEK::Vector3D<HEK::NativeEndian> *>(addr);
                    values[0] = static_cast<double>(vector.i);
                    values[1] = static_cast<double>(vector.j);
                    values[2] = static_cast<double>(vector.k);
                    addr += sizeof(vector);
                    values += 3;
                    break;
                }

                case VALUE_TYPE_EULER2D: {
                    const auto &vector = *reinterpret_cast<const HEK::Euler2D<HEK::NativeEndian> *>(addr);
                    values[0] = static_cast<double>(vector.yaw);
                    values[1] = static_cast<double>(vector.pitch);
                    addr += sizeof(vector);
                    values += 2;
                    break;
                }

                case VALUE_TYPE_EULER3D: {
                    const auto &vector = *reinterpret_cast<const HEK::Euler3D<HEK::NativeEndian> *>(addr);
                    values[0] = static_cast<double>(vector.yaw);
                    values[1] = static_cast<double>(vector.pitch);
                    values[2] = static_cast<double>(vector.roll);
                    addr += sizeof(vector);
                    values += 3;
                    break;
                }

                case VALUE_TYPE_PLANE2D: {
                    const auto &plane = *reinterpret_cast<const HEK::Plane2D<HEK::NativeEndian> *>(addr);
                    values[0] = static_cast<double>(plane.vector.i);
                    values[1] = static_cast<double>(plane.vector.j);
                    values[2] = static_cast<double>(plane.w);
                    addr += sizeof(plane);
                    values += 3;
                    break;
                }

                case VALUE_TYPE_PLANE3D: {
                    const auto &plane = *reinterpret_cast<const HEK::Plane3D<HEK::NativeEndian> *>(addr);
                    values[0] = static_cast<double>(plane.vector.i);
                    values[1] = static_cast<double>(plane.vector.j);
                    values[2] = static_cast<double>(plane.vector.k);
                    values[3] = static_cast<double>(plane.w);
                    addr += sizeof(plane);
                    values += 4;
                    break;
                }

                case VALUE_TYPE_POINT2D: {
                    const auto &point = *reinterpret_cast<const HEK::Point2D<HEK::NativeEndian> *>(addr);
                    values[0] = static_cast<double>(point.x);
                    values[1] = static_cast<double>(point.y);
                    addr += sizeof(point);
                    values += 2;
                    break;
                }

                case VALUE_TYPE_POINT2DINT: {
                    const auto &point = *reinterpret_cast<const HEK::Point2DInt<HEK::NativeEndian> *>(addr);
                    values[0] = static_cast<std::int64_t>(point.x);
                    values[1] = static_cast<std::int64_t>(point.y);
                    addr += sizeof(point);
                    values += 2;
                    break;
                }

                case VALUE_TYPE_POINT3D: {
                    const auto &point = *reinterpret_cast<const HEK::Point3D<HEK::NativeEndian> *>(addr);
                    values[0] = static_cast<double>(point.x);
                    values[1] = static_cast<double>(point.y);
                    values[2] = static_cast<double>(point.z);
                    addr += sizeof(point);
                    values += 3;
                    break;
                }

                case VALUE_TYPE_MATRIX: {
                    auto &matrix = *reinterpret_cast<const HEK::Matrix<HEK::NativeEndian> *>(addr);
                    values[0] = static_cast<double>(matrix.matrix[0][0]);
                    values[1] = static_cast<double>(matrix.matrix[0][1]);
                    values[2] = static_cast<double>(matrix.matrix[0][2]);
                    values[3] = static_cast<double>(matrix.matrix[1][0]);
                    values[4] = static_cast<double>(matrix.matrix[1][1]);
                    values[5] = static_cast<double>(matrix.matrix[1][2]);
                    values[6] = static_cast<double>(matrix.matrix[2][0]);
                    values[7] = static_cast<double>(matrix.matrix[2][1]);
                    values[8] = static_cast<double>(matrix.matrix[2][2]);
                    addr += sizeof(matrix);
                    values += 9;
                    break;
                }

                case VALUE_TYPE_QUATERNION: {
                    const auto &quaternion = *reinterpret_cast<const HEK::Quaternion<HEK::NativeEndian> *>(addr);
                    values[0] = static_cast<double>(quaternion.i);
                    values[1] = static_cast<double>(quaternion.j);
                    values[2] = static_cast<double>(quaternion.k);
                    values[3] = static_cast<double>(quaternion.w);
                    addr += sizeof(quaternion);
                    values += 4;
                    break;
                }

                case VALUE_TYPE_REFLEXIVE:
                case VALUE_TYPE_DEPENDENCY:
                case VALUE_TYPE_TAGID:
                case VALUE_TYPE_TAGSTRING:
                case VALUE_TYPE_TAGDATAOFFSET:
                case VALUE_TYPE_GROUP_START:
                    break;
            }
        }
    }

    void ParserStructValue::set_values(const Number *values) noexcept {
        auto *addr = reinterpret_cast<std::byte *>(this->address);
        for(std::size_t i = 0; i < this->count; i++) {
            switch(this->type) {
                case VALUE_TYPE_INT8:
                    *reinterpret_cast<std::int8_t *>(addr) = std::get<std::int64_t>(*values);
                    addr += sizeof(std::int8_t);
                    values++;
                    break;

                case VALUE_TYPE_UINT8:
                    *reinterpret_cast<std::uint8_t *>(addr) = std::get<std::int64_t>(*values);
                    addr += sizeof(std::uint8_t);
                    values++;
                    break;

                case VALUE_TYPE_INT16:
                    *reinterpret_cast<std::int16_t *>(addr) = std::get<std::int64_t>(*values);
                    addr += sizeof(std::int16_t);
                    values++;
                    break;

                case VALUE_TYPE_BITMASK:
                case VALUE_TYPE_ENUM:
                case VALUE_TYPE_UINT16:
                case VALUE_TYPE_INDEX:
                    *reinterpret_cast<std::uint16_t *>(addr) = std::get<std::int64_t>(*values);
                    addr += sizeof(std::uint16_t);
                    values++;
                    break;

                case VALUE_TYPE_INT32:
                    *reinterpret_cast<std::int32_t *>(addr) = std::get<std::int64_t>(*values);
                    addr += sizeof(std::int32_t);
                    values++;
                    break;

                case VALUE_TYPE_UINT32:
                    *reinterpret_cast<std::uint32_t *>(addr) = std::get<std::int64_t>(*values);
                    addr += sizeof(std::uint32_t);
                    values++;
                    break;

                case VALUE_TYPE_COLORARGBINT: {
                    auto &color = *reinterpret_cast<HEK::ColorARGBInt *>(addr);
                    color.alpha = static_cast<std::uint8_t>(std::get<std::int64_t>(values[0]));
                    color.red = static_cast<std::uint8_t>(std::get<std::int64_t>(values[1]));
                    color.green = static_cast<std::uint8_t>(std::get<std::int64_t>(values[2]));
                    color.blue = static_cast<std::uint8_t>(std::get<std::int64_t>(values[3]));
                    addr += sizeof(color);
                    values += 4;
                    break;
                }

                case VALUE_TYPE_RECTANGLE2D: {
                    auto &rectangle = *reinterpret_cast<HEK::Rectangle2D<HEK::NativeEndian> *>(addr);
                    rectangle.top = static_cast<std::int16_t>(std::get<std::int64_t>(values[0]));
                    rectangle.left = static_cast<std::int16_t>(std::get<std::int64_t>(values[1]));
                    rectangle.bottom = static_cast<std::int16_t>(std::get<std::int64_t>(values[2]));
                    rectangle.right = static_cast<std::int16_t>(std::get<std::int64_t>(values[3]));
                    addr += sizeof(rectangle);
                    values += 4;
                    break;
                }

                case VALUE_TYPE_FLOAT:
                case VALUE_TYPE_FRACTION:
                case VALUE_TYPE_ANGLE:
                    *reinterpret_cast<float *>(addr) = static_cast<float>(std::get<double>(*values));
                    addr += sizeof(float);
                    values++;
                    break;

                case VALUE_TYPE_COLORARGB: {
                    auto &color = *reinterpret_cast<HEK::ColorARGB<HEK::NativeEndian> *>(addr);
                    color.alpha = static_cast<float>(std::get<double>(values[0]));
                    color.red = static_cast<float>(std::get<double>(values[1]));
                    color.green = static_cast<float>(std::get<double>(values[2]));
                    color.blue = static_cast<float>(std::get<double>(values[3]));
                    addr += sizeof(color);
                    values += 4;
                    break;
                }

                case VALUE_TYPE_COLORRGB: {
                    auto &color = *reinterpret_cast<HEK::ColorRGB<HEK::NativeEndian> *>(addr);
                    color.red = static_cast<float>(std::get<double>(values[0]));
                    color.green = static_cast<float>(std::get<double>(values[1]));
                    color.blue = static_cast<float>(std::get<double>(values[2]));
                    addr += sizeof(color);
                    values += 3;
                    break;
                }

                case VALUE_TYPE_VECTOR2D: {
                    auto &vector = *reinterpret_cast<HEK::Vector2D<HEK::NativeEndian> *>(addr);
                    vector.i = static_cast<float>(std::get<double>(values[0]));
                    vector.j = static_cast<float>(std::get<double>(values[1]));
                    addr += sizeof(vector);
                    values += 2;
                    break;
                }

                case VALUE_TYPE_VECTOR3D: {
                    auto &vector = *reinterpret_cast<HEK::Vector3D<HEK::NativeEndian> *>(addr);
                    vector.i = static_cast<float>(std::get<double>(values[0]));
                    vector.j = static_cast<float>(std::get<double>(values[1]));
                    vector.k = static_cast<float>(std::get<double>(values[2]));
                    addr += sizeof(vector);
                    values += 3;
                    break;
                }

                case VALUE_TYPE_EULER2D: {
                    auto &vector = *reinterpret_cast<HEK::Euler2D<HEK::NativeEndian> *>(addr);
                    vector.yaw = static_cast<float>(std::get<double>(values[0]));
                    vector.pitch = static_cast<float>(std::get<double>(values[1]));
                    addr += sizeof(vector);
                    values += 2;
                    break;
                }

                case VALUE_TYPE_EULER3D: {
                    auto &vector = *reinterpret_cast<HEK::Euler3D<HEK::NativeEndian> *>(addr);
                    vector.yaw = static_cast<float>(std::get<double>(values[0]));
                    vector.pitch = static_cast<float>(std::get<double>(values[1]));
                    vector.roll = static_cast<float>(std::get<double>(values[2]));
                    addr += sizeof(vector);
                    values += 3;
                    break;
                }

                case VALUE_TYPE_PLANE2D: {
                    auto &plane = *reinterpret_cast<HEK::Plane2D<HEK::NativeEndian> *>(addr);
                    plane.vector.i = static_cast<float>(std::get<double>(values[0]));
                    plane.vector.j = static_cast<float>(std::get<double>(values[1]));
                    plane.w = static_cast<float>(std::get<double>(values[2]));
                    addr += sizeof(plane);
                    values += 3;
                    break;
                }

                case VALUE_TYPE_PLANE3D: {
                    auto &plane = *reinterpret_cast<HEK::Plane3D<HEK::NativeEndian> *>(addr);
                    plane.vector.i = static_cast<float>(std::get<double>(values[0]));
                    plane.vector.j = static_cast<float>(std::get<double>(values[1]));
                    plane.vector.k = static_cast<float>(std::get<double>(values[2]));
                    plane.w = static_cast<float>(std::get<double>(values[3]));
                    addr += sizeof(plane);
                    values += 4;
                    break;
                }

                case VALUE_TYPE_POINT2D: {
                    auto &point = *reinterpret_cast<HEK::Point2D<HEK::NativeEndian> *>(addr);
                    point.x = static_cast<float>(std::get<double>(values[0]));
                    point.y = static_cast<float>(std::get<double>(values[1]));
                    addr += sizeof(point);
                    values += 2;
                    break;
                }

                case VALUE_TYPE_POINT2DINT: {
                    auto &point = *reinterpret_cast<HEK::Point2DInt<HEK::NativeEndian> *>(addr);
                    point.x = static_cast<std::int16_t>(std::get<std::int64_t>(values[0]));
                    point.y = static_cast<std::int16_t>(std::get<std::int64_t>(values[1]));
                    addr += sizeof(point);
                    values += 2;
                    break;
                }

                case VALUE_TYPE_POINT3D: {
                    auto &point = *reinterpret_cast<HEK::Point3D<HEK::NativeEndian> *>(addr);
                    point.x = static_cast<float>(std::get<double>(values[0]));
                    point.y = static_cast<float>(std::get<double>(values[1]));
                    point.z = static_cast<float>(std::get<double>(values[2]));
                    addr += sizeof(point);
                    values += 3;
                    break;
                }

                case VALUE_TYPE_MATRIX: {
                    auto &matrix = *reinterpret_cast<HEK::Matrix<HEK::NativeEndian> *>(addr);
                    matrix.matrix[0][0] = static_cast<float>(std::get<double>(values[0]));
                    matrix.matrix[0][1] = static_cast<float>(std::get<double>(values[1]));
                    matrix.matrix[0][2] = static_cast<float>(std::get<double>(values[2]));
                    matrix.matrix[1][0] = static_cast<float>(std::get<double>(values[3]));
                    matrix.matrix[1][1] = static_cast<float>(std::get<double>(values[4]));
                    matrix.matrix[1][2] = static_cast<float>(std::get<double>(values[5]));
                    matrix.matrix[2][0] = static_cast<float>(std::get<double>(values[6]));
                    matrix.matrix[2][1] = static_cast<float>(std::get<double>(values[7]));
                    matrix.matrix[2][2] = static_cast<float>(std::get<double>(values[8]));
                    addr += sizeof(matrix);
                    values += 9;
                    break;
                }

                case VALUE_TYPE_QUATERNION: {
                    auto &quaternion = *reinterpret_cast<HEK::Quaternion<HEK::NativeEndian> *>(addr);
                    quaternion.i = static_cast<float>(std::get<double>(values[0]));
                    quaternion.j = static_cast<float>(std::get<double>(values[1]));
                    quaternion.k = static_cast<float>(std::get<double>(values[2]));
                    quaternion.w = static_cast<float>(std::get<double>(values[3]));
                    addr += sizeof(quaternion);
                    values += 4;
                    break;
                }

                case VALUE_TYPE_REFLEXIVE:
                case VALUE_TYPE_DEPENDENCY:
                case VALUE_TYPE_TAGSTRING:
                case VALUE_TYPE_TAGDATAOFFSET:
                case VALUE_TYPE_TAGID:
                case VALUE_TYPE_GROUP_START:
                    eprintf_error("Tried to use set_values() with a type that doesn't have values");
                    std::terminate();
            }
        }
    }

    void ParserStructValue::set_values(const std::vector<ParserStructValue::Number> &values) noexcept {
        return this->set_values(values.data());
    }

    std::unique_ptr<ParserStruct> ParserStruct::parse_hek_tag_file(const std::byte *data, std::size_t data_size, bool postprocess) {
        const auto *header = reinterpret_cast<const HEK::TagFileHeader *>(data);
        HEK::TagFileHeader::validate_header(header, data_size);

        #define DO_TAG_CLASS(class_struct, fourcc) case TagFourCC::fourcc: { \
            return std::make_unique<Parser::class_struct>(Invader::Parser::class_struct::parse_hek_tag_file(data, data_size, postprocess)); \
        }

        switch(header->tag_fourcc) {
            DO_BASED_ON_TAG_CLASS

            case Invader::HEK::TagFourCC::TAG_FOURCC_NONE:
            case Invader::HEK::TagFourCC::TAG_FOURCC_NULL:
            case Invader::HEK::TagFourCC::TAG_FOURCC_SPHEROID:
                break;
        }

        eprintf_error("Unknown tag class %s", tag_fourcc_to_extension(header->tag_fourcc));
        throw InvalidTagDataException();

        #undef DO_TAG_CLASS
    }

    std::unique_ptr<ParserStruct> ParserStruct::generate_base_struct(TagFourCC tag_class) {
        #define DO_TAG_CLASS(class_struct, fourcc) case TagFourCC::fourcc: { \
            return std::unique_ptr<ParserStruct>(new class_struct()); \
        }

        switch(tag_class) {
            DO_BASED_ON_TAG_CLASS

            case Invader::HEK::TagFourCC::TAG_FOURCC_NONE:
            case Invader::HEK::TagFourCC::TAG_FOURCC_NULL:
            case Invader::HEK::TagFourCC::TAG_FOURCC_SPHEROID:
                break;
        }

        return nullptr;

        #undef DO_TAG_CLASS
    }

    std::vector<TagFourCC> ParserStruct::all_tag_classes(bool exclude_subclasses) {
        std::vector<TagFourCC> classes;

        #define DO_TAG_CLASS(class_struct, fourcc) classes.emplace_back(TagFourCC::fourcc);
        DO_BASED_ON_TAG_CLASS;

        // Remove subclasses
        if(!exclude_subclasses) {
            for(std::size_t i = 0; i < classes.size(); i++) {
                if(classes[i] == TagFourCC::TAG_FOURCC_ITEM || classes[i] == TagFourCC::TAG_FOURCC_OBJECT || classes[i] == TagFourCC::TAG_FOURCC_UNIT || classes[i] == TagFourCC::TAG_FOURCC_DEVICE || classes[i] == TagFourCC::TAG_FOURCC_SHADER) {
                    classes.erase(classes.begin() + i--);
                }
            }
        }

        return classes;
    }

    bool ParserStruct::has_title() const {
        return false;
    }

    const char *ParserStruct::title() const {
        return nullptr;
    }

    std::size_t ParserStruct::refactor_reference(const File::TagFilePath &from, const File::TagFilePath &to) {
        return this->refactor_reference(from.path.c_str(), from.fourcc, to.path.c_str(), to.fourcc);
    }

    std::size_t ParserStruct::refactor_references(const std::vector<std::pair<File::TagFilePath, File::TagFilePath>> &replacements) {
        std::size_t total = 0;
        for(auto &i : replacements) {
            total += this->refactor_reference(i.first, i.second);
        }
        return total;
    }

    bool ParserStruct::check_for_invalid_references(bool null_references) {
        auto &values = this->get_values();
        bool result = false;
        for(auto &i : values) {
            switch(i.get_type()) {
                case ParserStructValue::ValueType::VALUE_TYPE_DEPENDENCY: {
                    auto &dep = i.get_dependency();
                    auto &allowed_classes = i.get_allowed_classes(); // get allowed classes
                    if(allowed_classes.size() >= 1 && !dep.path.empty()) { // do we even have any?
                        bool valid = false;
                        for(auto &c : allowed_classes) {
                            if(c == dep.tag_fourcc) {
                                valid = true;
                                break;
                            }
                        }
                        if(!valid) {
                            if(null_references) {
                                dep.tag_fourcc = allowed_classes[0]; // set it
                                dep.path.clear(); // clear the path
                            }
                            result = true;
                        }
                    }
                    break;
                }
                case ParserStructValue::ValueType::VALUE_TYPE_REFLEXIVE: {
                    auto count = i.get_array_size();
                    for(std::size_t c = 0; c < count && (!result || null_references); c++) { // continue until result is true, unless we're nulling references
                        result = i.get_object_in_array(c).check_for_invalid_references(null_references) || result;
                    }
                    break;
                }
                default:
                    break;
            }

            // If we're done, stop
            if(!null_references && result) {
                break;
            }
        }
        return result;
    }

    bool ParserStruct::compare(const ParserStruct *what, bool precision, bool ignore_volatile, std::list<std::string> *differences) const {
        return this->compare(what, precision, ignore_volatile, differences, 1);
    }

    static constexpr bool too_different(double a, double b) noexcept {
        double delta = FLOAT_EPSILON;

        auto max_discrepency = a * delta;
        if(max_discrepency < 0.0) {
            max_discrepency *= -1.0;
        }

        auto difference = (a - b);
        if(difference < 0.0) {
            difference *= -1.0;
        }

        // (a-b) > delta && b is not in [a - dA, a + dA]
        return difference > delta && ((b > a + max_discrepency) || (b < a - max_discrepency));
    }

    static_assert(too_different(1, 1) == false);
    static_assert(too_different(1.000025, 1) == false);
    static_assert(too_different(1.01, 1) == true);
    static_assert(too_different(-1, 1) == true);
    static_assert(too_different(-0.000025, 0.000025) == false);
    static_assert(too_different(0.000025, -0.000025) == false);

    bool ParserStruct::compare(const ParserStruct *what, bool precision, bool ignore_volatile, std::list<std::string> *differences_array, std::size_t depth) const {
        // Different struct name
        if(typeid(this) != typeid(what)) {
            if(differences_array != nullptr) {
                differences_array->emplace_back(std::string(this->struct_name()) + " is not a " + std::string(what->struct_name()));
            }
            return false;
        }

        auto &this_value = *this;

        // Make sure these are the same
        auto &v_this = this->get_values();
        auto &v_other = what->get_values();

        auto vt_size = v_this.size();
        auto vo_size = v_other.size();

        if(vt_size != vo_size) {
            eprintf_error("Value count is different for the same struct type");
            std::terminate();
        }

        bool should_continue = true;
        bool is_different = false;

        static constexpr const std::size_t depth_spacing_amount = 4;

        auto generate_depth_spacing = [](std::size_t depth) {
            char depth_spacing[512] = {};
            for(std::size_t d = 0; d < sizeof(depth_spacing) && d < depth * depth_spacing_amount; d++) {
                depth_spacing[d] = ' ';
            }
            return std::string(depth_spacing);
        };

        for(std::size_t v = 0; v < vt_size && should_continue; v++) {
            auto &vt = v_this[v];
            auto &vo = v_other[v];

            auto vt_type = vt.get_type();
            auto vo_type = vo.get_type();

            char difference_text[512] = {};

            auto complain = [&is_different, &should_continue, &differences_array, &vt, &this_value, &difference_text, &depth](std::optional<std::size_t> index = std::nullopt) {
                is_different = true;

                if(differences_array != nullptr) {
                    const char *lp = difference_text[0] == 0 ? "" : " (";
                    const char *rp = difference_text[0] == 0 ? "" : ")";

                    char depth_spacing[512] = {};
                    for(std::size_t d = 0; d < sizeof(depth_spacing) - 1 && d < depth * depth_spacing_amount; d++) {
                        depth_spacing[d] = ' ';
                    }

                    char message[512];
                    if(index.has_value()) {
                        std::snprintf(message, sizeof(message), "%s%s::%s#%zu is different%s%s%s", depth_spacing, this_value.struct_name(), vt.get_member_name(), *index, lp, difference_text, rp);
                    }
                    else {
                        std::snprintf(message, sizeof(message), "%s%s::%s is different%s%s%s", depth_spacing, this_value.struct_name(), vt.get_member_name(), lp, difference_text, rp);
                    }
                    differences_array->emplace_back(message);
                }
                else {
                    should_continue = false;
                }
            };

            // Check the type
            if(vt_type != vo_type) {
                eprintf_error("Type is different for the same struct type's values");
                std::terminate();
            }

            // Ignore volatile values
            if(ignore_volatile && vt.is_volatile()) {
                continue;
            }

            switch(vt_type) {
                case ParserStructValue::ValueType::VALUE_TYPE_GROUP_START:
                    break;
                case ParserStructValue::ValueType::VALUE_TYPE_TAGDATAOFFSET: {
                    auto &vt_data = vt.get_data();
                    auto &vo_data = vo.get_data();

                    // Check if the data is different
                    if(vt_data != vo_data) {
                        complain();

                        if(differences_array != nullptr) {
                            auto indent = generate_depth_spacing(depth + 1);
                            bool size_different = vt_data.size() != vo_data.size();

                            if(size_different) {
                                std::snprintf(difference_text, sizeof(difference_text), "%ssize: %zu != %zu", indent.c_str(), vt_data.size(), vo_data.size());
                                differences_array->emplace_back(difference_text);
                            }

                            auto vt_crc32 = ~crc32(0, vt_data.data(), vt_data.size());
                            auto vo_crc32 = ~crc32(0, vo_data.data(), vo_data.size());
                            bool crc32_different = vt_crc32 != vo_crc32;
                            std::snprintf(difference_text, sizeof(difference_text), "%scrc32: 0x%08X %s 0x%08X%s", indent.c_str(), vt_crc32, crc32_different ? "!=" : "==", vo_crc32, crc32_different ? "" : " (collision)");
                            differences_array->emplace_back(difference_text);
                        }
                    }
                    break;
                }
                case ParserStructValue::ValueType::VALUE_TYPE_DEPENDENCY: {
                    auto &a = vt.get_dependency();
                    auto &b = vo.get_dependency();
                    if(a != b) {
                        std::snprintf(difference_text, sizeof(difference_text), "'%s.%s' != '%s.%s'", File::halo_path_to_preferred_path(a.path).c_str(),
                                                                                                      HEK::tag_fourcc_to_extension(a.tag_fourcc),
                                                                                                      File::halo_path_to_preferred_path(b.path).c_str(),
                                                                                                      HEK::tag_fourcc_to_extension(b.tag_fourcc));
                        complain();
                    }
                    break;
                }
                case ParserStructValue::ValueType::VALUE_TYPE_TAGSTRING: {
                    const char *a = vt.get_string();
                    const char *b = vo.get_string();
                    if(std::strcmp(a, b) != 0) {
                        std::snprintf(difference_text, sizeof(difference_text), "'%s' != '%s'", a, b);
                        complain();
                    }
                    break;
                }
                case ParserStructValue::ValueType::VALUE_TYPE_ENUM: {
                    auto vt_values = vt.get_values();
                    auto vo_values = vo.get_values();

                    assert(vt_values.size() == vo_values.size() && vt_values.size() == 1);

                    const char *a, *b;
                    auto a_value = static_cast<int>(std::get<std::int64_t>(vt_values[0]));
                    auto b_value = static_cast<int>(std::get<std::int64_t>(vo_values[0]));

                    if(a_value != b_value) {
                        try {
                            a = vt.read_enum();
                        }
                        catch(std::exception &) {
                            a = "unknown_enum_value?";
                        }

                        try {
                            b = vo.read_enum();
                        }
                        catch(std::exception &) {
                            b = "unknown_enum_value?";
                        }

                        if(std::strcmp(a, b) != 0) {
                            std::snprintf(difference_text, sizeof(difference_text), "'%s' [%i] != '%s' [%i]", a, a_value, b, b_value);
                            complain();
                        }
                    }
                    break;
                }
                case ParserStructValue::ValueType::VALUE_TYPE_REFLEXIVE: {
                    auto vt_count = vt.get_array_size();
                    auto vo_count = vo.get_array_size();
                    if(vt_count != vo_count) {
                        std::snprintf(difference_text, sizeof(difference_text), "different sizes [%zu != %zu]", vt_count, vo_count);
                        complain();
                    }
                    else {
                        bool complained = false;

                        // Hold onto this for now so we can move stuff around
                        std::size_t current_difference_index = differences_array == nullptr ? 0 : differences_array->size();

                        depth++;

                        for(std::size_t i = 0; i < vt_count && should_continue; i++) {
                            const auto &vt_struct = vt.get_object_in_array(i);
                            const auto &vo_struct = vo.get_object_in_array(i);

                            // This will be filled if we're doing a verbose check
                            std::list<std::string> differences_this;

                            bool same = vt_struct.compare(&vo_struct, precision, ignore_volatile, differences_array == nullptr ? nullptr : &differences_this, depth + 1);
                            if(!same) {
                                complain(i);
                                complained = true;

                                if(differences_array != nullptr) {
                                    for(auto &i : differences_this) {
                                        differences_array->emplace_back(std::move(i));
                                    }
                                }
                            }
                        }

                        depth--;

                        // Complain some more.
                        if(complained) {
                            complain();

                            // If we have differences and we're doing verbose output, move the last element to where the bottom was before comparison
                            if(differences_array && current_difference_index < differences_array->size()) {
                                auto to_here = differences_array->begin();
                                for(std::size_t i = 0; i < current_difference_index; i++) {
                                    to_here++;
                                }

                                auto length = differences_array->size();
                                auto end = to_here;
                                for(std::size_t i = current_difference_index; i + 1 < length; i++) {
                                    end++;
                                }
                                auto last_str = std::move(*end);
                                differences_array->erase(end);
                                differences_array->insert(to_here, last_str);
                            }
                        }
                    }
                    break;
                }
                case ParserStructValue::ValueType::VALUE_TYPE_BITMASK: {
                    auto enums = vt.list_enum();
                    bool same = true;
                    std::list<std::string> differences;

                    for(auto &i : enums) {
                        // Append if different
                        auto a = vt.read_bitfield(i);
                        auto b = vo.read_bitfield(i);

                        if(a != b) {
                            std::snprintf(difference_text, sizeof(difference_text), "%s: %i != %i", i, a, b);
                            same = false;
                            if(differences_array != nullptr) {
                                differences.emplace_back(difference_text);
                            }
                        }
                    }

                    if(!same) {
                        difference_text[0] = 0;
                        complain();

                        if(differences_array != nullptr) {
                            auto spacing = generate_depth_spacing(depth + 1);
                            for(auto &i : differences) {
                                differences_array->emplace_back(spacing + i);
                            }
                        }
                    }

                    break;
                }
                default: {
                    auto vt_v = vt.get_values();
                    auto vo_v = vo.get_values();

                    // Go through each value to find a difference
                    auto value_count = vt_v.size();
                    auto fmt = vt.get_number_format();

                    std::list<std::string> differences_this_value;

                    bool differences_found = false;

                    for(std::size_t i = 0; i < value_count; i++) {
                        const char *prefix = "";

                        // TODO: Refactor this???
                        if(value_count > 1) {
                            if(vt.is_bounds()) {
                                prefix = i == 0 ? "f: " : "t: ";
                            }
                            else switch(vt_type) {
                                case ParserStructValue::VALUE_TYPE_COLORARGB:
                                case ParserStructValue::VALUE_TYPE_COLORARGBINT:
                                    switch(i % 4) {
                                        case 0:
                                            prefix = "a: ";
                                            break;
                                        case 1:
                                            prefix = "r: ";
                                            break;
                                        case 2:
                                            prefix = "g: ";
                                            break;
                                        case 3:
                                            prefix = "b: ";
                                            break;
                                    }
                                    break;
                                case ParserStructValue::VALUE_TYPE_COLORRGB:
                                    switch(i % 3) {
                                        case 0:
                                            prefix = "r: ";
                                            break;
                                        case 1:
                                            prefix = "g: ";
                                            break;
                                        case 2:
                                            prefix = "b: ";
                                            break;
                                    }
                                    break;
                                case ParserStructValue::VALUE_TYPE_EULER2D:
                                    switch(i % 2) {
                                        case 0:
                                            prefix = "y: ";
                                            break;
                                        case 1:
                                            prefix = "p: ";
                                            break;
                                    }
                                    break;
                                case ParserStructValue::VALUE_TYPE_EULER3D:
                                    switch(i % 2) {
                                        case 0:
                                            prefix = "y: ";
                                            break;
                                        case 1:
                                            prefix = "p: ";
                                            break;
                                        case 2:
                                            prefix = "r: ";
                                            break;
                                    }
                                    break;
                                case ParserStructValue::VALUE_TYPE_PLANE2D:
                                    switch(i % 3) {
                                        case 0:
                                            prefix = "i: ";
                                            break;
                                        case 1:
                                            prefix = "j: ";
                                            break;
                                        case 2:
                                            prefix = "w: ";
                                            break;
                                    }
                                    break;
                                case ParserStructValue::VALUE_TYPE_PLANE3D:
                                case ParserStructValue::VALUE_TYPE_QUATERNION:
                                    switch(i % 4) {
                                        case 0:
                                            prefix = "i: ";
                                            break;
                                        case 1:
                                            prefix = "j: ";
                                            break;
                                        case 2:
                                            prefix = "k: ";
                                            break;
                                        case 3:
                                            prefix = "w: ";
                                            break;
                                    }
                                    break;
                                case ParserStructValue::VALUE_TYPE_POINT2D:
                                case ParserStructValue::VALUE_TYPE_POINT2DINT:
                                    switch(i % 2) {
                                        case 0:
                                            prefix = "x: ";
                                            break;
                                        case 1:
                                            prefix = "y: ";
                                            break;
                                    }
                                    break;
                                case ParserStructValue::VALUE_TYPE_POINT3D:
                                    switch(i % 3) {
                                        case 0:
                                            prefix = "x: ";
                                            break;
                                        case 1:
                                            prefix = "y: ";
                                            break;
                                        case 2:
                                            prefix = "z: ";
                                            break;
                                    }
                                    break;
                                case ParserStructValue::VALUE_TYPE_VECTOR2D:
                                    switch(i % 2) {
                                        case 0:
                                            prefix = "i: ";
                                            break;
                                        case 1:
                                            prefix = "j: ";
                                            break;
                                    }
                                    break;
                                case ParserStructValue::VALUE_TYPE_VECTOR3D:
                                    switch(i % 3) {
                                        case 0:
                                            prefix = "i: ";
                                            break;
                                        case 1:
                                            prefix = "j: ";
                                            break;
                                        case 2:
                                            prefix = "k: ";
                                            break;
                                    }
                                    break;
                                case ParserStructValue::VALUE_TYPE_RECTANGLE2D:
                                    switch(i % 4) {
                                        case 0:
                                            prefix = "x0: ";
                                            break;
                                        case 1:
                                            prefix = "y0: ";
                                            break;
                                        case 2:
                                            prefix = "x1: ";
                                            break;
                                        case 3:
                                            prefix = "y1: ";
                                            break;
                                    }
                                    break;
                                case ParserStructValue::VALUE_TYPE_MATRIX:
                                    switch(i % 9) {
                                        case 0:
                                            prefix = "x0: ";
                                            break;
                                        case 1:
                                            prefix = "y0: ";
                                            break;
                                        case 2:
                                            prefix = "z0: ";
                                            break;
                                        case 3:
                                            prefix = "x1: ";
                                            break;
                                        case 4:
                                            prefix = "y1: ";
                                            break;
                                        case 5:
                                            prefix = "z1: ";
                                            break;
                                        case 6:
                                            prefix = "x2: ";
                                            break;
                                        case 7:
                                            prefix = "y2: ";
                                            break;
                                        case 8:
                                            prefix = "z2: ";
                                            break;
                                    }
                                    break;
                                default:
                                    break;
                            }
                        }

                        // Append if different
                        if(vt_v[i] != vo_v[i]) {
                            if(fmt == ParserStructValue::NumberFormat::NUMBER_FORMAT_FLOAT) {
                                double multiplier = (vt_type == ParserStructValue::ValueType::VALUE_TYPE_ANGLE || vt_type == ParserStructValue::ValueType::VALUE_TYPE_EULER2D || vt_type == ParserStructValue::ValueType::VALUE_TYPE_EULER3D) ? (180.0 / HALO_PI) : 1.0;

                                union float_type_punner_9000 {
                                    float f;
                                    std::uint32_t u;
                                };

                                float_type_punner_9000 t_float = { static_cast<float>(std::get<double>(vt_v[i])) };
                                float_type_punner_9000 v_float = { static_cast<float>(std::get<double>(vo_v[i])) };

                                // If accounting for slight precision errors, skip
                                if(precision && !too_different(t_float.f, v_float.f)) {
                                    continue;
                                }

                                std::snprintf(difference_text, sizeof(difference_text), "%s%f [0x%08X] != %f [0x%08X]", prefix, t_float.f * multiplier, t_float.u, v_float.f * multiplier, v_float.u);
                            }
                            else if(fmt == ParserStructValue::NumberFormat::NUMBER_FORMAT_INT) {
                                std::snprintf(difference_text, sizeof(difference_text), "%s%lli != %lli", prefix, static_cast<long long>(std::get<std::int64_t>(vt_v[i])), static_cast<long long>(std::get<std::int64_t>(vo_v[i])));
                            }

                            differences_found = true;

                            if(value_count > 1) {
                                differences_this_value.emplace_back(difference_text);
                            }
                        }
                    }

                    // Set the first byte to 0, making it an empty string
                    if(value_count > 1) {
                        difference_text[0] = 0;
                    }

                    // Did we find a difference?
                    if(differences_found) {
                        complain();

                        // Append everything
                        if(value_count > 1 && differences_array != nullptr) {
                            std::string depth_spacing_str = generate_depth_spacing(depth + 1);
                            for(auto &d : differences_this_value) {
                                differences_array->emplace_back(depth_spacing_str + d);
                            }
                        }
                    }
                    break;
                }
            }
        }

        return !is_different;
    }

    bool ParserStruct::check_for_broken_enums(bool reset_enums) {
        auto &values = this->get_values();
        bool result = false;
        for(auto &i : values) {
            switch(i.get_type()) {
                case ParserStructValue::ValueType::VALUE_TYPE_ENUM: {
                    try {
                        auto *enum_val = i.read_enum();
                        bool inside = false;

                        for(auto &k : i.list_enum()) {
                            if(std::strcmp(k, enum_val) == 0) {
                                inside = true;
                                break;
                            }
                        }

                        if(!inside) {
                            throw std::exception(); // throw - it's not valid
                        }
                    }
                    catch(std::exception &) {
                        if(reset_enums) {
                            i.write_enum(i.list_enum()[0]);
                        }
                        result = true;
                    }
                    break;
                }
                case ParserStructValue::ValueType::VALUE_TYPE_REFLEXIVE: {
                    auto count = i.get_array_size();
                    for(std::size_t c = 0; c < count && (!result || reset_enums); c++) { // continue until result is true, unless we're nulling references
                        result = i.get_object_in_array(c).check_for_broken_enums(reset_enums) || result;
                    }
                    break;
                }
                default:
                    break;
            }

            // If we're done, stop
            if(!reset_enums && result) {
                break;
            }
        }
        return result;
    }

    std::vector<ParserStructValue> &ParserStruct::get_values() {
        if(!this->values.has_value()) {
            this->values = this->get_values_internal();
        }
        return *this->values;
    }

    ParserStruct::ParserStruct(const ParserStruct &copy) noexcept : cache_formatted(copy.cache_formatted) {}
    ParserStruct::ParserStruct(ParserStruct &&move) noexcept : cache_formatted(move.cache_formatted) {}
    ParserStruct &ParserStruct::operator=(const ParserStruct &copy) noexcept {
        this->cache_formatted = copy.cache_formatted;
        return *this;
    }
}
