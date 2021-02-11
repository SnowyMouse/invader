// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/tag/parser/parser_struct.hpp>
#include <invader/tag/hek/header.hpp>
#include <invader/file/file.hpp>

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
            case VALUE_TYPE_BITMASK:
            case VALUE_TYPE_TAGSTRING:
            case VALUE_TYPE_TAGDATAOFFSET:
            case VALUE_TYPE_ENUM:
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
                case VALUE_TYPE_BITMASK:
                case VALUE_TYPE_TAGSTRING:
                case VALUE_TYPE_TAGDATAOFFSET:
                case VALUE_TYPE_ENUM:
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

        #define DO_TAG_CLASS(class_struct, class_int) case TagClassInt::class_int: { \
            return std::make_unique<Parser::class_struct>(Invader::Parser::class_struct::parse_hek_tag_file(data, data_size, postprocess)); \
        }

        switch(header->tag_class_int) {
            DO_BASED_ON_TAG_CLASS

            case Invader::HEK::TagClassInt::TAG_CLASS_INVADER_SCENARIO:
            case Invader::HEK::TagClassInt::TAG_CLASS_NONE:
            case Invader::HEK::TagClassInt::TAG_CLASS_NULL:
            case Invader::HEK::TagClassInt::TAG_CLASS_INVADER_FONT:
            case Invader::HEK::TagClassInt::TAG_CLASS_INVADER_UI_WIDGET_DEFINITION:
            case Invader::HEK::TagClassInt::TAG_CLASS_INVADER_UNIT_HUD_INTERFACE:
            case Invader::HEK::TagClassInt::TAG_CLASS_INVADER_WEAPON_HUD_INTERFACE:
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

            case Invader::HEK::TagClassInt::TAG_CLASS_NONE:
            case Invader::HEK::TagClassInt::TAG_CLASS_NULL:
            case Invader::HEK::TagClassInt::TAG_CLASS_INVADER_SCENARIO:
            case Invader::HEK::TagClassInt::TAG_CLASS_INVADER_FONT:
            case Invader::HEK::TagClassInt::TAG_CLASS_INVADER_UI_WIDGET_DEFINITION:
            case Invader::HEK::TagClassInt::TAG_CLASS_INVADER_UNIT_HUD_INTERFACE:
            case Invader::HEK::TagClassInt::TAG_CLASS_INVADER_WEAPON_HUD_INTERFACE:
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

    bool ParserStruct::has_title() const {
        return false;
    }

    const char *ParserStruct::title() const {
        return nullptr;
    }
    
    std::size_t ParserStruct::refactor_reference(const File::TagFilePath &from, const File::TagFilePath &to) {
        return this->refactor_reference(from.path.c_str(), from.class_int, to.path.c_str(), to.class_int);
    }
    
    std::size_t ParserStruct::refactor_references(const std::vector<std::pair<File::TagFilePath, File::TagFilePath>> &replacements) {
        std::size_t total = 0;
        for(auto &i : replacements) {
            total += this->refactor_reference(i.first, i.second);
        }
        return total;
    }
    
    bool ParserStruct::compare(const ParserStruct *what, bool precision, bool ignore_volatile, bool verbose) const {
        return this->compare(what, precision, ignore_volatile, verbose, 0);
    }
}
