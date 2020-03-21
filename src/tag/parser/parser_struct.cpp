// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/tag/parser/parser_struct.hpp>
#include <invader/tag/hek/header.hpp>

namespace Invader::Parser {
    ParserStructValue::ParserStructValue(
        const char *       name,
        const char *       member_name,
        const char *       comment,
        Dependency *       dependency,
        const TagClassInt *allowed_classes,
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
        const char *member_name,
        const char *comment,
        void *      object,
        ValueType   type,
        const char *unit,
        std::size_t count,
        bool        bounds,
        bool        read_only
    ) : name(name),
        member_name(member_name),
        comment(comment),
        type(type),
        address(object),
        count(count),
        bounds(bounds),
        unit(unit),
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
            case VALUE_TYPE_BITMASK:
            case VALUE_TYPE_TAGSTRING:
            case VALUE_TYPE_TAGDATAOFFSET:
            case VALUE_TYPE_ENUM:
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
                case VALUE_TYPE_BITMASK:
                case VALUE_TYPE_TAGSTRING:
                case VALUE_TYPE_TAGDATAOFFSET:
                case VALUE_TYPE_ENUM:
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
                case VALUE_TYPE_BITMASK:
                case VALUE_TYPE_TAGSTRING:
                case VALUE_TYPE_TAGDATAOFFSET:
                case VALUE_TYPE_ENUM:
                    eprintf_error("Tried to use set_values() with a type that doesn't have values");
                    std::terminate();
            }
        }
    }

    void ParserStructValue::set_values(const std::vector<ParserStructValue::Number> &values) noexcept {
        return this->set_values(values.data());
    }

    #define DO_BASED_ON_TAG_CLASS DO_TAG_CLASS(Actor, TAG_CLASS_ACTOR) \
                                  DO_TAG_CLASS(ActorVariant, TAG_CLASS_ACTOR_VARIANT) \
                                  DO_TAG_CLASS(Antenna, TAG_CLASS_ANTENNA) \
                                  DO_TAG_CLASS(Biped, TAG_CLASS_BIPED) \
                                  DO_TAG_CLASS(Bitmap, TAG_CLASS_BITMAP) \
                                  DO_TAG_CLASS(CameraTrack, TAG_CLASS_CAMERA_TRACK) \
                                  DO_TAG_CLASS(ColorTable, TAG_CLASS_COLOR_TABLE) \
                                  DO_TAG_CLASS(Contrail, TAG_CLASS_CONTRAIL) \
                                  DO_TAG_CLASS(DamageEffect, TAG_CLASS_DAMAGE_EFFECT) \
                                  DO_TAG_CLASS(Decal, TAG_CLASS_DECAL) \
                                  DO_TAG_CLASS(DetailObjectCollection, TAG_CLASS_DETAIL_OBJECT_COLLECTION) \
                                  DO_TAG_CLASS(Device, TAG_CLASS_DEVICE) \
                                  DO_TAG_CLASS(DeviceControl, TAG_CLASS_DEVICE_CONTROL) \
                                  DO_TAG_CLASS(DeviceLightFixture, TAG_CLASS_DEVICE_LIGHT_FIXTURE) \
                                  DO_TAG_CLASS(DeviceMachine, TAG_CLASS_DEVICE_MACHINE) \
                                  DO_TAG_CLASS(Dialogue, TAG_CLASS_DIALOGUE) \
                                  DO_TAG_CLASS(Effect, TAG_CLASS_EFFECT) \
                                  DO_TAG_CLASS(Equipment, TAG_CLASS_EQUIPMENT) \
                                  DO_TAG_CLASS(ExtendedBitmap, TAG_CLASS_EXTENDED_BITMAP) \
                                  DO_TAG_CLASS(ExtendedSound, TAG_CLASS_EXTENDED_SOUND) \
                                  DO_TAG_CLASS(Flag, TAG_CLASS_FLAG) \
                                  DO_TAG_CLASS(Fog, TAG_CLASS_FOG) \
                                  DO_TAG_CLASS(Font, TAG_CLASS_FONT) \
                                  DO_TAG_CLASS(Garbage, TAG_CLASS_GARBAGE) \
                                  DO_TAG_CLASS(GBXModel, TAG_CLASS_GBXMODEL) \
                                  DO_TAG_CLASS(Globals, TAG_CLASS_GLOBALS) \
                                  DO_TAG_CLASS(Glow, TAG_CLASS_GLOW) \
                                  DO_TAG_CLASS(GrenadeHUDInterface, TAG_CLASS_GRENADE_HUD_INTERFACE) \
                                  DO_TAG_CLASS(HUDGlobals, TAG_CLASS_HUD_GLOBALS) \
                                  DO_TAG_CLASS(HUDMessageText, TAG_CLASS_HUD_MESSAGE_TEXT) \
                                  DO_TAG_CLASS(HUDNumber, TAG_CLASS_HUD_NUMBER) \
                                  DO_TAG_CLASS(InputDeviceDefaults, TAG_CLASS_INPUT_DEVICE_DEFAULTS) \
                                  DO_TAG_CLASS(Item, TAG_CLASS_ITEM) \
                                  DO_TAG_CLASS(ItemCollection, TAG_CLASS_ITEM_COLLECTION) \
                                  DO_TAG_CLASS(LensFlare, TAG_CLASS_LENS_FLARE) \
                                  DO_TAG_CLASS(Light, TAG_CLASS_LIGHT) \
                                  DO_TAG_CLASS(LightVolume, TAG_CLASS_LIGHT_VOLUME) \
                                  DO_TAG_CLASS(Lightning, TAG_CLASS_LIGHTNING) \
                                  DO_TAG_CLASS(MaterialEffects, TAG_CLASS_MATERIAL_EFFECTS) \
                                  DO_TAG_CLASS(Meter, TAG_CLASS_METER) \
                                  DO_TAG_CLASS(ModelAnimations, TAG_CLASS_MODEL_ANIMATIONS) \
                                  DO_TAG_CLASS(ModelCollisionGeometry, TAG_CLASS_MODEL_COLLISION_GEOMETRY) \
                                  DO_TAG_CLASS(MultiplayerScenarioDescription, TAG_CLASS_MULTIPLAYER_SCENARIO_DESCRIPTION) \
                                  DO_TAG_CLASS(Object, TAG_CLASS_OBJECT) \
                                  DO_TAG_CLASS(Particle, TAG_CLASS_PARTICLE) \
                                  DO_TAG_CLASS(ParticleSystem, TAG_CLASS_PARTICLE_SYSTEM) \
                                  DO_TAG_CLASS(Physics, TAG_CLASS_PHYSICS) \
                                  DO_TAG_CLASS(Placeholder, TAG_CLASS_PLACEHOLDER) \
                                  DO_TAG_CLASS(PointPhysics, TAG_CLASS_POINT_PHYSICS) \
                                  DO_TAG_CLASS(Projectile, TAG_CLASS_PROJECTILE) \
                                  DO_TAG_CLASS(Scenario, TAG_CLASS_SCENARIO) \
                                  DO_TAG_CLASS(ScenarioStructureBSP, TAG_CLASS_SCENARIO_STRUCTURE_BSP) \
                                  DO_TAG_CLASS(Scenery, TAG_CLASS_SCENERY) \
                                  DO_TAG_CLASS(Shader, TAG_CLASS_SHADER) \
                                  DO_TAG_CLASS(ShaderTransparentChicago, TAG_CLASS_SHADER_TRANSPARENT_CHICAGO) \
                                  DO_TAG_CLASS(ShaderTransparentChicagoExtended, TAG_CLASS_SHADER_TRANSPARENT_CHICAGO_EXTENDED) \
                                  DO_TAG_CLASS(ShaderEnvironment, TAG_CLASS_SHADER_ENVIRONMENT) \
                                  DO_TAG_CLASS(ShaderModel, TAG_CLASS_SHADER_MODEL) \
                                  DO_TAG_CLASS(ShaderTransparentGeneric, TAG_CLASS_SHADER_TRANSPARENT_GENERIC) \
                                  DO_TAG_CLASS(ShaderTransparentGlass, TAG_CLASS_SHADER_TRANSPARENT_GLASS) \
                                  DO_TAG_CLASS(ShaderTransparentMeter, TAG_CLASS_SHADER_TRANSPARENT_METER) \
                                  DO_TAG_CLASS(ShaderTransparentPlasma, TAG_CLASS_SHADER_TRANSPARENT_PLASMA) \
                                  DO_TAG_CLASS(ShaderTransparentWater, TAG_CLASS_SHADER_TRANSPARENT_WATER) \
                                  DO_TAG_CLASS(Sky, TAG_CLASS_SKY) \
                                  DO_TAG_CLASS(Sound, TAG_CLASS_SOUND) \
                                  DO_TAG_CLASS(SoundScenery, TAG_CLASS_SOUND_SCENERY) \
                                  DO_TAG_CLASS(SoundEnvironment, TAG_CLASS_SOUND_ENVIRONMENT) \
                                  DO_TAG_CLASS(SoundLooping, TAG_CLASS_SOUND_LOOPING) \
                                  DO_TAG_CLASS(StringList, TAG_CLASS_STRING_LIST) \
                                  DO_TAG_CLASS(TagCollection, TAG_CLASS_TAG_COLLECTION) \
                                  DO_TAG_CLASS(UnicodeStringList, TAG_CLASS_UNICODE_STRING_LIST) \
                                  DO_TAG_CLASS(UnitHUDInterface, TAG_CLASS_UNIT_HUD_INTERFACE) \
                                  DO_TAG_CLASS(Unit, TAG_CLASS_UNIT) \
                                  DO_TAG_CLASS(Vehicle, TAG_CLASS_VEHICLE) \
                                  DO_TAG_CLASS(VirtualKeyboard, TAG_CLASS_VIRTUAL_KEYBOARD) \
                                  DO_TAG_CLASS(Weapon, TAG_CLASS_WEAPON) \
                                  DO_TAG_CLASS(WeaponHUDInterface, TAG_CLASS_WEAPON_HUD_INTERFACE) \
                                  DO_TAG_CLASS(WeatherParticleSystem, TAG_CLASS_WEATHER_PARTICLE_SYSTEM) \
                                  DO_TAG_CLASS(Wind, TAG_CLASS_WIND) \
                                  DO_TAG_CLASS(TagCollection, TAG_CLASS_UI_WIDGET_COLLECTION) \
                                  DO_TAG_CLASS(UIWidgetDefinition, TAG_CLASS_UI_WIDGET_DEFINITION) \

    std::unique_ptr<ParserStruct> ParserStruct::parse_hek_tag_file(const std::byte *data, std::size_t data_size, bool postprocess) {
        const auto *header = reinterpret_cast<const HEK::TagFileHeader *>(data);
        HEK::TagFileHeader::validate_header(header, data_size);

        #define DO_TAG_CLASS(class_struct, class_int) case TagClassInt::class_int: { \
            return std::make_unique<Parser::class_struct>(Invader::Parser::class_struct::parse_hek_tag_file(data, data_size, postprocess)); \
        }

        switch(header->tag_class_int) {
            DO_BASED_ON_TAG_CLASS

            case Invader::HEK::TagClassInt::TAG_CLASS_PREFERENCES_NETWORK_GAME:
            case Invader::HEK::TagClassInt::TAG_CLASS_SPHEROID:
            case Invader::HEK::TagClassInt::TAG_CLASS_CONTINUOUS_DAMAGE_EFFECT:
            case Invader::HEK::TagClassInt::TAG_CLASS_EXTENDED_SCENARIO:
            case Invader::HEK::TagClassInt::TAG_CLASS_MODEL:
            case Invader::HEK::TagClassInt::TAG_CLASS_NONE:
            case Invader::HEK::TagClassInt::TAG_CLASS_NULL:
            case Invader::HEK::TagClassInt::TAG_CLASS_NEW_FONT:
            case Invader::HEK::TagClassInt::TAG_CLASS_NEW_UI_WIDGET_DEFINITION:
            case Invader::HEK::TagClassInt::TAG_CLASS_NEW_UNIT_HUD_INTERFACE:
            case Invader::HEK::TagClassInt::TAG_CLASS_NEW_WEAPON_HUD_INTERFACE:
            case Invader::HEK::TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GLSL:
                break;
        }

        eprintf_error("Unknown tag class %s", tag_class_to_extension(header->tag_class_int));
        throw InvalidTagDataException();

        #undef DO_TAG_CLASS
    }

    std::unique_ptr<ParserStruct> ParserStruct::generate_base_struct(TagClassInt tag_class) {
        #define DO_TAG_CLASS(class_struct, class_int) case TagClassInt::class_int: { \
            return std::unique_ptr<ParserStruct>(new class_struct()); \
        }

        switch(tag_class) {
            DO_BASED_ON_TAG_CLASS

            case Invader::HEK::TagClassInt::TAG_CLASS_PREFERENCES_NETWORK_GAME:
            case Invader::HEK::TagClassInt::TAG_CLASS_SPHEROID:
            case Invader::HEK::TagClassInt::TAG_CLASS_CONTINUOUS_DAMAGE_EFFECT:
            case Invader::HEK::TagClassInt::TAG_CLASS_MODEL:
            case Invader::HEK::TagClassInt::TAG_CLASS_NONE:
            case Invader::HEK::TagClassInt::TAG_CLASS_NULL:
            case Invader::HEK::TagClassInt::TAG_CLASS_EXTENDED_SCENARIO:
            case Invader::HEK::TagClassInt::TAG_CLASS_NEW_FONT:
            case Invader::HEK::TagClassInt::TAG_CLASS_NEW_UI_WIDGET_DEFINITION:
            case Invader::HEK::TagClassInt::TAG_CLASS_NEW_UNIT_HUD_INTERFACE:
            case Invader::HEK::TagClassInt::TAG_CLASS_NEW_WEAPON_HUD_INTERFACE:
            case Invader::HEK::TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GLSL:
                break;
        }

        return nullptr;

        #undef DO_TAG_CLASS
    }

    std::vector<TagClassInt> ParserStruct::all_tag_classes(bool exclude_subclasses) {
        std::vector<TagClassInt> classes;

        #define DO_TAG_CLASS(class_struct, class_int) classes.emplace_back(TagClassInt::class_int);
        DO_BASED_ON_TAG_CLASS;

        // Remove subclasses
        if(!exclude_subclasses) {
            for(std::size_t i = 0; i < classes.size(); i++) {
                if(classes[i] == TagClassInt::TAG_CLASS_ITEM || classes[i] == TagClassInt::TAG_CLASS_OBJECT || classes[i] == TagClassInt::TAG_CLASS_UNIT || classes[i] == TagClassInt::TAG_CLASS_DEVICE || classes[i] == TagClassInt::TAG_CLASS_SHADER) {
                    classes.erase(classes.begin() + i--);
                }
            }
        }

        return classes;
    }

    bool ParserStruct::has_title() {
        return false;
    }

    const char *ParserStruct::title() {
        return nullptr;
    }
}
