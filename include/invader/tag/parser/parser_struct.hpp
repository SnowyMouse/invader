// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__PARSER__PARSER_STRUCT_HPP
#define INVADER__TAG__PARSER__PARSER_STRUCT_HPP

#include <vector>
#include <cstddef>
#include <optional>
#include <variant>
#include "../hek/definition.hpp"

namespace Invader {
    class BuildWorkload;
}

namespace Invader::Parser {
    class ParserStructValue {
    public:
        enum ValueType {
            // Integer stuff
            VALUE_TYPE_INT8,
            VALUE_TYPE_UINT8,
            VALUE_TYPE_INT16,
            VALUE_TYPE_UINT16,
            VALUE_TYPE_INT32,
            VALUE_TYPE_UINT32,
            VALUE_TYPE_COLOR_ARGB8,

            // Float stuff
            VALUE_TYPE_FLOAT,
            VALUE_TYPE_ANGLE,
            VALUE_TYPE_COLOR_ARGB,
            VALUE_TYPE_COLOR_RGB,
            VALUE_TYPE_VECTOR2D,
            VALUE_TYPE_VECTOR3D,
            VALUE_TYPE_PLANE2D,
            VALUE_TYPE_PLANE3D,
            VALUE_TYPE_POINT2D,
            VALUE_TYPE_POINT3D,
            VALUE_TYPE_QUATERNION,

            // Other stuff
            VALUE_TYPE_DATA,
            VALUE_TYPE_REFLEXIVE, // use templates for this maybe?
            VALUE_TYPE_DEPENDENCY,
            VALUE_TYPE_BITMASK
        };

        enum NumberFormat {
            NUMBER_FORMAT_FLOAT,
            NUMBER_FORMAT_INT,
            NUMBER_FORMAT_NONE
        };

        using Number = std::variant<std::int64_t, double>;

        /**
         * Get the value count
         * @return value count
         */
        std::size_t get_value_count() const noexcept;

        /**
         * Get the number format
         * @return number format
         */
        NumberFormat get_number_format() const noexcept;

        /**
         * Get the values
         * @param values values to write to; must point to at least get_value_count() values
         */
        void get_values(Number *values) const noexcept;

        /**
         * Set the values
         * @param values values to read from; must point to at least get_value_count() values
         */
        void set_values(const Number *values) const noexcept;

        /**
         * Get the value type used
         * @return value type
         */
        ValueType get_type() const noexcept {
            return this->type;
        }

        /**
         * Get the name of the value
         * @return name of the value
         */
        const char *get_name() const noexcept {
            return this->name;
        }

    private:
        ValueType type;
        const char *name;
        std::byte *address;
    };

    struct ParserStruct {
        /**
         * Get whether or not the data is formatted for cache files.
         * @return true if data is formatted for cache files
         */
        bool is_cache_formatted() const noexcept { return this->cache_formatted; };

        /**
         * Format the tag to be used in HEK tags.
         */
        virtual void cache_deformat() = 0;

        /**
         * Compile the tag to be used in cache files.
         * @param workload     workload struct to use
         * @param tag_index    tag index to use in the workload
         * @param struct_index struct index to use in the workload
         * @param bsp          BSP index to use
         * @param offset       struct offset
         */
        virtual void compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::optional<std::size_t> bsp = std::nullopt, std::size_t offset = 0) = 0;

        /**
         * Convert the struct into HEK tag data to be built into a cache file.
         * @param  generate_header_class generate a cache file header with the class, too
         * @param  clear_on_save         clear data as it's being saved (reduces memory usage but you can't work on the tag anymore)
         * @return cache file data
         */
        virtual std::vector<std::byte> generate_hek_tag_data(std::optional<TagClassInt> generate_header_class = std::nullopt, bool clear_on_save = false) = 0;

        /**
         * Refactor the tag reference, replacing all references with the given reference. Paths must use Halo path separators.\n")
         * @param from_path  Path to look for\n")
         * @param from_class Class to look for\n")
         * @param to_path    Path to replace with\n")
         * @param to_class   Class to replace with\n")
         * @return           number of references replaced\n")
         */
        virtual std::size_t refactor_reference(const char *from_path, TagClassInt from_class, const char *to_path, TagClassInt to_class) = 0;

        virtual ~ParserStruct() = default;
    protected:
        bool cache_formatted = false;
    };
}

#endif
