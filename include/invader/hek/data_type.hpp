// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__HEK__DATA_TYPE_HPP
#define INVADER__HEK__DATA_TYPE_HPP

#include <string>
#include <vector>
#include <cmath>

#include "../error.hpp"
#include "constants.hpp"
#include "pad.hpp"
#include "fourcc.hpp"
#include "endian.hpp"
#include "../printf.hpp"

/**
 * Calculate the required amount of padding to make a size divisible by a set number of bytes
 * @param  size size
 * @return      padding required
 */
#define REQUIRED_PADDING_N_BYTES(size, n) static_cast<std::size_t>((~(size - 1)) & (n - 1))

/**
 * Calculate the required amount of padding to make a size divisible by 32 bits
 * @param  size size
 * @return      padding required
 */
#define REQUIRED_PADDING_32_BIT(size) REQUIRED_PADDING_N_BYTES(size, 4)

/**
 * Convert degrees to radians
 * @param  deg Degrees to convert from
 * @return     Radians
 */
#define DEGREES_TO_RADIANS(deg) static_cast<float>(deg * HALO_PI / 180.0F)

/**
 * Convert radians to degrees
 * @param  rad Radians to convert from
 * @return     Degrees
 */
#define RADIANS_TO_DEGREES(rad) static_cast<float>(rad * 180.0F / HALO_PI)

namespace Invader::HEK {
    using Pointer = std::uint32_t;
    using Pointer64 = std::uint64_t;
    using TagEnum = std::uint16_t;
    using Angle = float;
    using Fraction = float;
    using Index = std::uint16_t;
    
    /**
     * Check if the number is power-of-two
     * @param value value to check
     * @return      true if the value is power-of-two; false if not
     */
    template <typename T> [[maybe_unused]] static constexpr inline bool is_power_of_two(T value) noexcept {
        while(value > 1) {
            if(value & 1) {
                return false;
            }
            value >>= 1;
        }
        return value & 1;
    }

    #define NULL_INDEX (0xFFFF)

    /**
     * This refers to a tag ID when compiled.
     */
    union TagID {
        /**
         * Full ID
         */
        std::uint32_t id;

        /**
         * Index of the tag ID in the tag array
         */
        std::uint16_t index;

        /**
         * If the full ID is this value, then the tag ID is null
         */
        static const std::uint32_t NULL_ID = 0xFFFFFFFF;

        /**
         * Check if the tag ID is null
         * @return true if the tag ID is null
         */
        bool is_null() const {
            return this->id == NULL_ID;
        }

        static TagID null_tag_id() {
            return { NULL_ID };
        }
        
        bool operator==(const TagID &other) const noexcept {
            return other.index == this->index && other.is_null() == this->is_null();
        }
        
        bool operator!=(const TagID &other) const noexcept {
            return other.index != this->index || other.is_null() != this->is_null();
        }
    };

    /**
     * Single 20 character string
     */
    struct TagString {
        char string[0x20] = {};

        /**
         * Check if the string overflows
         * @param  length optionally set the length to this
         * @return        true if the string overflows
         */
        bool overflows(std::size_t *length = nullptr) const noexcept {
            for(const char &c : this->string) {
                if(c == 0) {
                    if(length) {
                        *length = &c - this->string;
                    }
                    return false;
                }
            }
            return true;
        }

        TagString() = default;
        TagString(const TagString &copy) {
            std::size_t length = 0;
            if(copy.overflows(&length)) {
                eprintf_error("String overflow detected!!!");
                throw OutOfBoundsException();
            }
            std::copy(copy.string, copy.string + length, this->string);
            std::fill(this->string + length, this->string + sizeof(this->string), 0x0);
        }
        TagString &operator=(const TagString &copy) {
            std::size_t length = 0;
            if(copy.overflows(&length)) {
                eprintf_error("String overflow detected!!!");
                throw OutOfBoundsException();
            }
            std::copy(copy.string, copy.string + length, this->string);
            std::fill(this->string + length, this->string + sizeof(this->string), 0x0);
            return *this;
        }
        bool operator==(const TagString &other) const noexcept {
            return std::strncmp(other.string, this->string, sizeof(other.string)) == 0;
        }
        bool operator!=(const TagString &other) const noexcept {
            return std::strncmp(other.string, this->string, sizeof(other.string)) != 0;
        }
        bool operator >(const TagString &other) const noexcept {
            return std::strncmp(other.string, this->string, sizeof(other.string)) > 0;
        }
        bool operator <(const TagString &other) const noexcept {
            return std::strncmp(other.string, this->string, sizeof(other.string)) < 0;
        }
        bool operator>=(const TagString &other) const noexcept {
            return std::strncmp(other.string, this->string, sizeof(other.string)) >= 0;
        }
        bool operator<=(const TagString &other) const noexcept {
            return std::strncmp(other.string, this->string, sizeof(other.string)) <= 0;
        }
    };

    /**
     * Unsigned integer that uses the high-order bit as a flag
     */
    template<typename T> struct FlaggedInt {
        /**
         * Raw value
         */
        T value;

        static_assert(std::is_signed<T>() == false, "FlaggedInt takes an unsigned integer.");

        /**
         * Flag value
         */
        static constexpr T FLAG_BIT = (1 << (sizeof(T) * 8 - 1));

        /**
         * Maximum value of the integer
         */
        static constexpr T MAX_VALUE = FLAG_BIT - 1;

        /**
         * Create a null FlaggedInt
         * @return null FlaggedInt
         */
        static FlaggedInt<T> null() {
            return FlaggedInt<T> { (FLAG_BIT | MAX_VALUE) };
        }

        /**
         * Return true if the flag is set
         * @return true if flag is set
         */
        bool flag_value() const noexcept {
            return this->value & FLAG_BIT;
        }

        /**
         * Return true if this is a null value
         * @return true if this is a null value
         */
        bool is_null() const noexcept {
            return this->value == (FLAG_BIT | MAX_VALUE);
        }

        /**
         * Get the value of the integer
         * @return value of the integer
         */
        T int_value() const noexcept {
            return this->value & MAX_VALUE;
        }

        /**
         * Set the flag of the integer
         * @param new_flag_value value of the flag to set
         */
        void set_flag(bool new_flag_value) noexcept {
            this->value = this->int_value() | (FLAG_BIT * new_flag_value);
        }

        /**
         * Set the flag of the integer
         * @param new_int_value value of the integer to set
         */
        void set_value(T new_int_value) noexcept {
            this->value = (new_int_value & MAX_VALUE) | (this->flag_value());
        }

        operator T() const noexcept {
            return this->int_value();
        }

        bool operator ==(const FlaggedInt<T> &other) {
            return this->value == other.value;
        }
    };
    static_assert(sizeof(FlaggedInt<std::uint32_t>) == sizeof(std::uint32_t));
    static_assert(FlaggedInt<std::uint32_t>::MAX_VALUE == 0x7FFFFFFF);
    static_assert(FlaggedInt<std::uint32_t>::FLAG_BIT == 0x80000000);

    /**
     * Dependencies allow tags to reference other tags.
     */
    ENDIAN_TEMPLATE(EndianType) struct TagDependency {
        /** Tag class of the tag being depended upon */
        EndianType<TagFourCC> tag_fourcc;

        /** Pointer to tag path when compiled. */
        EndianType<Pointer> path_pointer;

        /** Length of the path in bytes, not including the null terminator. */
        EndianType<std::uint32_t> path_size;

        /** Tag ID when compiled. Otherwise equals 0xFFFFFFFF */
        EndianType<TagID> tag_id;

        ENDIAN_TEMPLATE(OtherEndian) operator TagDependency<OtherEndian>() const {
            TagDependency<OtherEndian> copy;
            COPY_THIS(tag_fourcc);
            COPY_THIS(path_pointer);
            COPY_THIS(path_size);
            COPY_THIS(tag_id);
            return copy;
        }
    };

    /**
     * Reflexives reference other tag blocks
     */
    template <template<typename> class EndianType, template<template<typename> typename> typename StructType> struct TagReflexive {
        /** Number of chunks */
        EndianType<std::uint32_t> count;

        /** Pointer to first block if compiled */
        EndianType<Pointer64> pointer;

        /** Little endian version of the struct type */
        using struct_type_little = StructType<LittleEndian>;

        /** Big endian version of the struct type */
        using struct_type_big = StructType<BigEndian>;

        /**
         * Return a pointer to the structs in data.
         * @param  data         reference to the data
         * @param  base_pointer base pointer to the beginning of the data
         * @return              pointer to the first struct or nullptr if count is 0
         * @throws              throw an exception if out of bounds
         */
        struct_type_little *get_structs(std::vector<std::byte> &data, Pointer base_pointer) const {
            if(count == 0) {
                return nullptr;
            }
            auto pointer = this->pointer.read();
            if(pointer < base_pointer) {
                throw OutOfBoundsException();
            }
            std::size_t offset = pointer - base_pointer;
            if(offset >= data.size() || offset + sizeof(struct_type_little) * this->count.read() > data.size()) {
                throw OutOfBoundsException();
            }
            return reinterpret_cast<struct_type_little *>(data.data() + offset);
        }

        operator TagReflexive<LittleEndian, StructType>() const {
            TagReflexive<LittleEndian, StructType> copy;
            COPY_THIS(count);
            copy.pointer = 0;
            copy.unknown = 0;
            return copy;
        }

        template <template<typename> class OtherEndian> operator TagReflexive<OtherEndian, StructType>() const {
            TagReflexive<OtherEndian, StructType> copy;
            COPY_THIS(count);
            COPY_THIS(pointer);
            COPY_THIS(unknown);
            return copy;
        }
    };
    static_assert(sizeof(TagReflexive<BigEndian, TagDependency>) == 0xC);

    /**
     * Rotation Matrix
     */
    ENDIAN_TEMPLATE(EndianType) struct Matrix {
        EndianType<float> matrix[3][3];

        ENDIAN_TEMPLATE(OtherEndian) operator Matrix<OtherEndian>() const {
            Matrix<OtherEndian> copy;
            COPY_THIS_ARRAY(matrix[0]);
            COPY_THIS_ARRAY(matrix[1]);
            COPY_THIS_ARRAY(matrix[2]);
            return copy;
        }
    };
    static_assert(sizeof(Matrix<BigEndian>) == 0x24);

    /**
     * Quaternion
     */
    ENDIAN_TEMPLATE(EndianType) struct Quaternion {
        EndianType<float> i;
        EndianType<float> j;
        EndianType<float> k;
        EndianType<float> w;

        ENDIAN_TEMPLATE(OtherEndian) operator Quaternion<OtherEndian>() const {
            Quaternion<OtherEndian> copy;
            COPY_THIS(i);
            COPY_THIS(j);
            COPY_THIS(k);
            COPY_THIS(w);
            return copy;
        }
        
        static const constexpr double NONNORMAL_THRESHOLD = 0.00001;
        
        float calculate_scale() const noexcept {
            return std::sqrt(i*i + j*j + k*k + w*w);
        }
        
        bool is_normalized() const noexcept {
            return std::fabs(1.0 - this->calculate_scale()) < NONNORMAL_THRESHOLD;
        }

        Quaternion<EndianType> normalize() const noexcept {
            // First let's get the distance
            float distance = std::sqrt(i*i + j*j + k*k + w*w);

            // If it's 0, we can't normalize it
            if(distance == 0.0F) {
                return { 0.0F, 0.0F, 0.0F, 1.0F };
            }

            // Find what we must multiply to get
            float m_distance = 1.0F / distance;

            // Now write and return the new values
            Quaternion<EndianType> copy;
            copy.i = i * m_distance;
            copy.j = j * m_distance;
            copy.k = k * m_distance;
            copy.w = w * m_distance;
            return copy;
        }
        
        bool operator ==(const Quaternion<EndianType> &other) const noexcept {
            return this->i == other.i && this->j == other.j && this->k == other.k && this->w == other.w;
        };
        
        bool operator !=(const Quaternion<EndianType> &other) const noexcept {
            return !(*this == other);
        };
    };
    static_assert(sizeof(Quaternion<BigEndian>) == 0x10);

    /**
     * RGB Color with alpha
     */
    ENDIAN_TEMPLATE(EndianType) struct ColorARGB {
        EndianType<float> alpha;
        EndianType<float> red;
        EndianType<float> green;
        EndianType<float> blue;

        ENDIAN_TEMPLATE(OtherEndian) operator ColorARGB<OtherEndian>() const {
            ColorARGB<OtherEndian> copy;
            COPY_THIS(alpha);
            COPY_THIS(red);
            COPY_THIS(green);
            COPY_THIS(blue);
            return copy;
        }
    };
    static_assert(sizeof(ColorARGB<BigEndian>) == 0x10);

    /**
     * Rectangle
     */
    ENDIAN_TEMPLATE(EndianType) struct Rectangle2D {
        EndianType<std::int16_t> top;
        EndianType<std::int16_t> left;
        EndianType<std::int16_t> bottom;
        EndianType<std::int16_t> right;

        ENDIAN_TEMPLATE(OtherEndian) operator Rectangle2D<OtherEndian>() const {
            Rectangle2D<OtherEndian> copy;
            COPY_THIS(top);
            COPY_THIS(left);
            COPY_THIS(bottom);
            COPY_THIS(right);
            return copy;
        }
    };
    static_assert(sizeof(Rectangle2D<BigEndian>) == 0x8);

    /**
     * RGB Color with alpha - This is stored as a 32-bit integer, so we're ordering it from lowest 8 bits to highest 8 bits
     */
    struct ColorARGBInt {
        std::uint8_t blue;
        std::uint8_t green;
        std::uint8_t red;
        std::uint8_t alpha;
    };
    static_assert(sizeof(ColorARGBInt) == 0x4);

    /**
     * RGB Color
     */
    ENDIAN_TEMPLATE(EndianType) struct ColorRGB {
        EndianType<float> red;
        EndianType<float> green;
        EndianType<float> blue;

        ENDIAN_TEMPLATE(OtherEndian) operator ColorRGB<OtherEndian>() const {
            ColorRGB<OtherEndian> copy;
            COPY_THIS(red);
            COPY_THIS(green);
            COPY_THIS(blue);
            return copy;
        }
    };
    static_assert(sizeof(ColorRGB<BigEndian>) == 0xC);

    /**
     * 3D Vector
     */
    ENDIAN_TEMPLATE(EndianType) struct Euler3D {
        EndianType<float> yaw;
        EndianType<float> pitch;
        EndianType<float> roll;

        ENDIAN_TEMPLATE(OtherEndian) operator Euler3D<OtherEndian>() const {
            Euler3D<OtherEndian> copy;
            COPY_THIS(yaw);
            COPY_THIS(pitch);
            COPY_THIS(roll);
            return copy;
        }
    };
    static_assert(sizeof(Euler3D<BigEndian>) == 0xC);

    /**
     * 2D Vector
     */
    ENDIAN_TEMPLATE(EndianType) struct Euler2D {
        EndianType<float> yaw;
        EndianType<float> pitch;

        ENDIAN_TEMPLATE(OtherEndian) operator Euler2D<OtherEndian>() const {
            Euler2D<OtherEndian> copy;
            COPY_THIS(yaw);
            COPY_THIS(pitch);
            return copy;
        }
    };
    static_assert(sizeof(Euler2D<BigEndian>) == 0x8);

    /**
     * 3D Vector
     */
    ENDIAN_TEMPLATE(EndianType) struct Vector3D {
        EndianType<float> i;
        EndianType<float> j;
        EndianType<float> k;

        ENDIAN_TEMPLATE(OtherEndian) operator Vector3D<OtherEndian>() const {
            Vector3D<OtherEndian> copy;
            COPY_THIS(i);
            COPY_THIS(j);
            COPY_THIS(k);
            return copy;
        }

        Vector3D<EndianType> operator*(float multiply) const {
            Vector3D<EndianType> copy;
            copy.i = this->i * multiply;
            copy.j = this->j * multiply;
            copy.k = this->k * multiply;
            return copy;
        }

        Vector3D<EndianType> operator+(const Vector3D<EndianType> &add) const {
            Vector3D<EndianType> copy;
            copy.i = this->i + add.i;
            copy.j = this->j + add.j;
            copy.k = this->k + add.k;
            return copy;
        }

        bool operator==(const Vector3D<EndianType> &other) const {
            return this->i == other.i && this->j == other.j && this->k == other.k;
        }
        bool operator!=(const Vector3D<EndianType> &other) const {
            return !(*this == other);
        }
        
        static const constexpr double NONNORMAL_THRESHOLD = 0.00001;
        
        float calculate_scale() const noexcept {
            return std::sqrt(i*i + j*j + k*k);
        }
        
        bool is_normalized() const noexcept {
            return std::fabs(1.0 - this->calculate_scale()) < NONNORMAL_THRESHOLD;
        }

        Vector3D<EndianType> normalize() const noexcept {
            // First let's get the distance
            float distance = std::sqrt(i*i + j*j + k*k);

            // If it's 0, we can't normalize it
            if(distance == 0.0F) {
                return { 0.0F, 0.0F, 1.0F };
            }

            // Find what we must multiply to get
            float m_distance = 1.0F / distance;

            // Now write and return the new values
            Vector3D<EndianType> copy;
            copy.i = i * m_distance;
            copy.j = j * m_distance;
            copy.k = k * m_distance;
            return copy;
        }
    };
    static_assert(sizeof(Vector3D<BigEndian>) == 0xC);

    /**
     * 2D Vector
     */
    ENDIAN_TEMPLATE(EndianType) struct Vector2D {
        EndianType<float> i;
        EndianType<float> j;

        ENDIAN_TEMPLATE(OtherEndian) operator Vector2D<OtherEndian>() const {
            Vector2D<OtherEndian> copy;
            COPY_THIS(i);
            COPY_THIS(j);
            return copy;
        }

        ENDIAN_TEMPLATE(OtherEndian) operator Vector3D<OtherEndian>() const {
            Vector3D<OtherEndian> copy;
            COPY_THIS(i);
            COPY_THIS(j);
            copy.k = 0.0F;
            return copy;
        }

        bool operator==(const Vector2D<EndianType> &other) const {
            return this->i == other.i && this->j == other.j;
        }
        bool operator!=(const Vector2D<EndianType> &other) const {
            return !(*this == other);
        }
        
        static const constexpr double NONNORMAL_THRESHOLD = 0.00001;
        
        float calculate_scale() const noexcept {
            return std::sqrt(i*i + j*j);
        }
        
        bool is_normalized() const noexcept {
            return std::fabs(1.0 - this->calculate_scale()) < NONNORMAL_THRESHOLD;
        }

        Vector2D<EndianType> normalize() const noexcept {
            // First let's get the distance
            float distance = std::sqrt(i*i + j*j);

            // If it's 0, we can't normalize it
            if(distance == 0.0F) {
                return { 0.0F, 1.0F };
            }

            // Find what we must multiply to get
            float m_distance = 1.0F / distance;

            // Now write and return the new values
            Vector2D<EndianType> copy;
            copy.i = i * m_distance;
            copy.j = j * m_distance;
            return copy;
        }
    };
    static_assert(sizeof(Vector2D<BigEndian>) == 0x8);

    /**
     * 3D Plane
     */
    ENDIAN_TEMPLATE(EndianType) struct Plane3D {
        Vector3D<EndianType> vector;
        EndianType<float> w;

        bool operator==(const Plane3D<EndianType> &other) const {
            return this->vector == other.vector && this->w == other.w;
        }
        bool operator!=(const Plane3D<EndianType> &other) const {
            return !(*this == other);
        }

        ENDIAN_TEMPLATE(OtherEndian) operator Plane3D<OtherEndian>() const {
            Plane3D<OtherEndian> copy;
            COPY_THIS(vector);
            COPY_THIS(w);
            return copy;
        }
    };
    static_assert(sizeof(Plane3D<BigEndian>) == 0x10);

    /**
     * 2D Plane
     */
    ENDIAN_TEMPLATE(EndianType) struct Plane2D {
        Vector2D<EndianType> vector;
        EndianType<float> w;

        bool operator==(const Plane2D<EndianType> &other) const {
            return this->vector == other.vector && this->w == other.w;
        }
        bool operator!=(const Plane2D<EndianType> &other) const {
            return !(*this == other);
        }

        ENDIAN_TEMPLATE(OtherEndian) operator Plane2D<OtherEndian>() const {
            Plane2D<OtherEndian> copy;
            COPY_THIS(vector);
            COPY_THIS(w);
            return copy;
        }
    };
    static_assert(sizeof(Plane2D<BigEndian>) == 0xC);

    /**
     * 2D Point
     */
    ENDIAN_TEMPLATE(EndianType) struct Point2D {
        EndianType<float> x;
        EndianType<float> y;

        ENDIAN_TEMPLATE(OtherEndian) operator Point2D<OtherEndian>() const {
            Point2D<OtherEndian> copy;
            COPY_THIS(x);
            COPY_THIS(y);
            return copy;
        }

        Point2D<EndianType> operator+(const Point2D<EndianType> &add) const {
            Point2D<EndianType> copy;
            copy.x = this->x + add.x;
            copy.y = this->y + add.y;
            return copy;
        }

        Point2D<EndianType> operator*(float multiply) const {
            Point2D<EndianType> copy;
            copy.x = this->x * multiply;
            copy.y = this->y * multiply;
            return copy;
        }

        Point2D<EndianType> operator/(float divide) const {
            Point2D<EndianType> copy;
            copy.x = this->x / divide;
            copy.y = this->y / divide;
            return copy;
        }
        
        bool operator==(const Point2D<EndianType> &other) const {
            return other.x == this->x && other.y == this->y;
        }
        
        bool operator!=(const Point2D<EndianType> &other) const {
            return !(*this == other);
        }

        /**
         * Get the distance from the plane
         * @param  cmp Plane to check
         * @return     Distance in world units (positive if in front, negative if behind, zero if neither)
         */
        float distance_from_plane(const Plane2D<EndianType> &plane) const {
            return ((plane.vector.i * this->x) + (plane.vector.j * this->y)) - plane.w;
        }
    };
    static_assert(sizeof(Point2D<BigEndian>) == 0x8);

    /**
     * 3D Point
     */
    ENDIAN_TEMPLATE(EndianType) struct Point3D {
        EndianType<float> x;
        EndianType<float> y;
        EndianType<float> z;

        ENDIAN_TEMPLATE(OtherEndian) operator Point3D<OtherEndian>() const {
            Point3D<OtherEndian> copy;
            COPY_THIS(x);
            COPY_THIS(y);
            COPY_THIS(z);
            return copy;
        }

        ENDIAN_TEMPLATE(OtherEndian) operator Vector3D<OtherEndian>() const {
            Vector3D<OtherEndian> copy;
            copy.i = this->x;
            copy.j = this->y;
            copy.k = this->z;
            return copy;
        }

        Point3D<EndianType> operator+(const Point3D<EndianType> &add) const {
            Point3D<EndianType> copy;
            copy.x = this->x + add.x;
            copy.y = this->y + add.y;
            copy.z = this->z + add.z;
            return copy;
        }

        Point3D<EndianType> operator+(const Vector3D<EndianType> &add) const {
            Point3D<EndianType> copy;
            copy.x = this->x + add.i;
            copy.y = this->y + add.j;
            copy.z = this->z + add.k;
            return copy;
        }

        Point3D<EndianType> operator+(float add) const {
            Point3D<EndianType> copy;
            copy.x = this->x + add;
            copy.y = this->y + add;
            copy.z = this->z + add;
            return copy;
        }

        Point3D<EndianType> operator*(float multiply) const {
            Point3D<EndianType> copy;
            copy.x = this->x * multiply;
            copy.y = this->y * multiply;
            copy.z = this->z * multiply;
            return copy;
        }

        Point3D<EndianType> operator/(float divide) const {
            Point3D<EndianType> copy;
            copy.x = this->x / divide;
            copy.y = this->y / divide;
            copy.z = this->z / divide;
            return copy;
        }

        bool operator ==(const Point3D<EndianType> &other) const {
            return this->x == other.x && this->y == other.y && this->z == other.z;
        }
        
        bool operator !=(const Point3D<EndianType> &other) const {
            return !(*this == other);
        }

        /**
         * Get the distance from the plane
         * @param  plane Plane to check
         * @return       Distance in world units (positive if in front, negative if behind, zero if neither)
         */
        float distance_from_plane(const Plane3D<EndianType> &plane) const {
            return ((plane.vector.i * this->x) + (plane.vector.j * this->y) + (plane.vector.k * this->z)) - plane.w;
        }

        /**
         * Get the distance from the plane
         * @param plane Plane to check
         * @return      distance from the plane in world units
         */
        float operator-(const Plane3D<EndianType> &plane) const {
            return this->distance_from_plane(plane);
        }

        /**
         * Calculate a non-normalized vector between two points
         * @param point Other point to check
         * @return      vector calculated
         */
        Vector3D<EndianType> operator-(const Point3D<EndianType> &point) const {
            Vector3D<EndianType> v;
            v.i = this->x - point.x;
            v.j = this->y - point.y;
            v.k = this->z - point.z;
            return v;
        }

        /**
         * Get the distance from the point squared. This is faster because no square root operation is used.
         * @param  point Point to check
         * @return       Distance in world units
         */
        float distance_from_point_squared(const Point3D<EndianType> &point) const {
            float x = point.x - this->x;
            float y = point.y - this->y;
            float z = point.z - this->z;
            return x*x + y*y + z*z;
        }

        /**
         * Get the distance from the point.
         * @param  point Point to check
         * @return       Distance in world units
         */
        float distance_from_point(const Point3D<EndianType> &point) const {
            return std::sqrt(this->distance_from_point_squared(point));
        }

        Point3D(const Vector3D<EndianType> &copy) : x(copy.i), y(copy.j), z(copy.k) {}
        Point3D(const Point3D<EndianType> &copy) = default;
        Point3D() = default;
    };
    static_assert(sizeof(Point3D<BigEndian>) == 0xC);

    /**
     * 2D Point
     */
    ENDIAN_TEMPLATE(EndianType) struct Point2DInt {
        EndianType<std::int16_t> x;
        EndianType<std::int16_t> y;

        ENDIAN_TEMPLATE(OtherEndian) operator Point2DInt<OtherEndian>() const {
            Point2DInt<OtherEndian> copy;
            COPY_THIS(x);
            COPY_THIS(y);
            return copy;
        }
    };
    static_assert(sizeof(Point2DInt<BigEndian>) == 0x4);

    /**
     * Bounds from one value to another
     */
    template <typename BoundsType> struct Bounds {
        BoundsType from;
        BoundsType to;

        template <typename NewType> operator Bounds<NewType>() const {
            Bounds<NewType> copy;
            COPY_THIS(from);
            COPY_THIS(to);
            return copy;
        }
        
        bool operator ==(const Bounds<BoundsType> &other) const {
            return this->from == other.from && this->to == other.to;
        }
    };
    static_assert(sizeof(Bounds<BigEndian<float>>) == 0x8);

    #define SINGLE_DEPENDENCY_STRUCT(name, dependency) ENDIAN_TEMPLATE(EndianType) struct name {\
        TagDependency<EndianType> dependency;\
        ENDIAN_TEMPLATE(NewType) operator name <NewType>() const noexcept {\
            name <NewType> copy = {};\
            COPY_THIS(dependency);\
            return copy;\
        }\
    }

    #define SINGLE_DEPENDENCY_PADDED_STRUCT(name, dependency, pad) ENDIAN_TEMPLATE(EndianType) struct name {\
        TagDependency<EndianType> dependency;\
        PAD(pad);\
        ENDIAN_TEMPLATE(NewType) operator name <NewType>() const noexcept {\
            name <NewType> copy = {};\
            COPY_THIS(dependency);\
            return copy;\
        }\
    }

    ENDIAN_TEMPLATE(EndianType) struct TagDataOffset {
        EndianType<std::uint32_t> size;
        LittleEndian<std::uint32_t> external;
        EndianType<std::uint32_t> file_offset;
        EndianType<Pointer64> pointer;

        ENDIAN_TEMPLATE(OtherType) operator TagDataOffset<OtherType>() const noexcept {
            TagDataOffset<OtherType> copy = {};
            COPY_THIS(size);
            COPY_THIS(external);
            COPY_THIS(file_offset);
            COPY_THIS(pointer);
            return copy;
        }
    };
    static_assert(sizeof(TagDataOffset<BigEndian>) == 0x14);

    /**
     * Node value; can hold a variety of different value types
     */
    union ScenarioScriptNodeValue {
        std::int8_t bool_int;
        std::int16_t short_int;
        std::int32_t long_int;
        float real;
        TagID tag_id;

        ScenarioScriptNodeValue() = default;
        ScenarioScriptNodeValue(const ScenarioScriptNodeValue &copy) = default;

        ScenarioScriptNodeValue(std::uint8_t v) {
            this->bool_int = v;
        }

        ScenarioScriptNodeValue(std::uint16_t v) {
            this->short_int = v;
        }

        ScenarioScriptNodeValue(std::uint32_t v) {
            this->long_int = v;
        }

        ScenarioScriptNodeValue(float v) {
            this->real = v;
        }

        ScenarioScriptNodeValue(TagID v) {
            this->tag_id = v;
        }

        void operator=(std::uint8_t v) {
            this->long_int = 0xFFFFFFFF;
            this->bool_int = v;
        }

        void operator=(std::uint16_t v) {
            this->long_int = 0xFFFFFFFF;
            this->short_int = v;
        }

        void operator=(std::uint32_t v) {
            this->long_int = v;
        }

        void operator=(float v) {
            this->real = v;
        }

        void operator=(TagID v) {
            this->tag_id = v;
        }
        
        bool operator==(const ScenarioScriptNodeValue &other) const noexcept {
            return this->long_int == other.long_int;
        }
        
        bool operator!=(const ScenarioScriptNodeValue &other) const noexcept {
            return this->long_int != other.long_int;
        }
    };

    Vector3D<NativeEndian> euler2d_to_vector(const Euler2D<NativeEndian> &rotation) noexcept;

    Quaternion<NativeEndian> vector_to_quaternion(const Vector3D<NativeEndian> &vector) noexcept;
    Quaternion<NativeEndian> euler_to_quaternion(const Euler3D<NativeEndian> &rotation) noexcept;
    Matrix<NativeEndian> quaternion_to_matrix(const Quaternion<NativeEndian> &rotation) noexcept;
    Matrix<NativeEndian> euler_to_matrix(const Euler3D<NativeEndian> &rotation) noexcept;

    Matrix<NativeEndian> multiply_matrix(const Matrix<NativeEndian> &rotation, float value) noexcept;
    Matrix<NativeEndian> multiply_matrix(const Matrix<NativeEndian> &rotation, const Matrix<NativeEndian> &value) noexcept;
    Matrix<NativeEndian> invert_matrix(const Matrix<NativeEndian> &rotation) noexcept;

    Vector3D<NativeEndian> add_vector(const Vector3D<NativeEndian> &vector, const Vector3D<NativeEndian> &value) noexcept;
    Vector3D<NativeEndian> multiply_vector(const Vector3D<NativeEndian> &vector, float value) noexcept;
    Vector3D<NativeEndian> rotate_vector(const Vector3D<NativeEndian> &vector, const Quaternion<NativeEndian> &rotation) noexcept;
    Vector3D<NativeEndian> rotate_vector(const Vector3D<NativeEndian> &vector, const Matrix<NativeEndian> &rotation) noexcept;

    ENDIAN_TEMPLATE(EndianType) struct ModelVertexCompressed;
    ENDIAN_TEMPLATE(EndianType) struct ModelVertexUncompressed;

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPMaterialCompressedRenderedVertex;
    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPMaterialUncompressedRenderedVertex;

    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPMaterialCompressedLightmapVertex;
    ENDIAN_TEMPLATE(EndianType) struct ScenarioStructureBSPMaterialUncompressedLightmapVertex;

    ModelVertexCompressed<NativeEndian> compress_model_vertex(const ModelVertexUncompressed<NativeEndian> &vertex) noexcept;
    ModelVertexUncompressed<NativeEndian> decompress_model_vertex(const ModelVertexCompressed<NativeEndian> &vertex) noexcept;
    
    std::uint32_t compress_vector(float i, float j, float k) noexcept;

    ScenarioStructureBSPMaterialCompressedRenderedVertex<NativeEndian> compress_sbsp_rendered_vertex(const ScenarioStructureBSPMaterialUncompressedRenderedVertex<NativeEndian> &vertex) noexcept;
    ScenarioStructureBSPMaterialUncompressedRenderedVertex<NativeEndian> decompress_sbsp_rendered_vertex(const ScenarioStructureBSPMaterialCompressedRenderedVertex<NativeEndian> &vertex) noexcept;

    ScenarioStructureBSPMaterialCompressedLightmapVertex<NativeEndian> compress_sbsp_lightmap_vertex(const ScenarioStructureBSPMaterialUncompressedLightmapVertex<NativeEndian> &vertex) noexcept;
    ScenarioStructureBSPMaterialUncompressedLightmapVertex<NativeEndian> decompress_sbsp_lightmap_vertex(const ScenarioStructureBSPMaterialCompressedLightmapVertex<NativeEndian> &vertex) noexcept;

    bool intersect_plane_with_points(const Plane3D<NativeEndian> &plane, const Point3D<NativeEndian> &point_a, const Point3D<NativeEndian> &point_b, Point3D<NativeEndian> *intersection = nullptr, float epsilon = 0.0001);

    inline float dot3(const Vector3D<NativeEndian> &vector_a, const Vector3D<NativeEndian> &vector_b) {
        return (vector_a.i * vector_b.i) + (vector_a.j * vector_b.j) + (vector_a.k * vector_b.k);
    }
    inline float dot3(const Vector3D<NativeEndian> &vector_a, const Point3D<NativeEndian> &point_b) {
        return (vector_a.i * point_b.x) + (vector_a.j * point_b.y) + (vector_a.k * point_b.z);
    }
}
#endif
