// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__PARSER__PARSER_STRUCT_HPP
#define INVADER__TAG__PARSER__PARSER_STRUCT_HPP

#include <vector>
#include <deque>
#include <cstddef>
#include <optional>
#include <variant>
#include <memory>
#include "../hek/definition.hpp"

namespace Invader {
    class BuildWorkload;
}

namespace Invader::File {
    struct TagFilePath;
}

namespace Invader::Parser {
    struct ParserStruct;

    struct Dependency {
        TagClassInt tag_class_int;
        std::string path;
        HEK::TagID tag_id = HEK::TagID::null_tag_id();
        
        bool operator==(const Dependency &other) const {
            return this->path == other.path && (this->path.size() == 0 || this->tag_class_int == other.tag_class_int);
        }
        
        bool operator!=(const Dependency &other) const {
            return !(*this == other);
        }
    };

    class ParserStructValue {
    public:
        enum ValueType {
            // Integer stuff
            VALUE_TYPE_INT8,
            VALUE_TYPE_UINT8,
            VALUE_TYPE_INT16,
            VALUE_TYPE_UINT16,
            VALUE_TYPE_INDEX,
            VALUE_TYPE_INT32,
            VALUE_TYPE_UINT32,
            VALUE_TYPE_COLORARGBINT,
            VALUE_TYPE_POINT2DINT,
            VALUE_TYPE_RECTANGLE2D,

            // Float stuff
            VALUE_TYPE_FLOAT,
            VALUE_TYPE_FRACTION,
            VALUE_TYPE_ANGLE,
            VALUE_TYPE_COLORARGB,
            VALUE_TYPE_COLORRGB,
            VALUE_TYPE_VECTOR2D,
            VALUE_TYPE_VECTOR3D,
            VALUE_TYPE_EULER2D,
            VALUE_TYPE_EULER3D,
            VALUE_TYPE_PLANE2D,
            VALUE_TYPE_PLANE3D,
            VALUE_TYPE_POINT2D,
            VALUE_TYPE_POINT3D,
            VALUE_TYPE_QUATERNION,
            VALUE_TYPE_MATRIX,

            // Other stuff
            VALUE_TYPE_REFLEXIVE, // use templates for this maybe?
            VALUE_TYPE_DEPENDENCY,
            VALUE_TYPE_TAGSTRING,
            VALUE_TYPE_TAGDATAOFFSET,
            VALUE_TYPE_ENUM,
            VALUE_TYPE_BITMASK,
            VALUE_TYPE_GROUP_START
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
         * Get the values
         * @return vector of values
         */
        std::vector<Number> get_values() const;

        /**
         * Set the values
         * @param values values to read from; must point to at least get_value_count() values
         */
        void set_values(const Number *values) noexcept;

        /**
         * Set the values
         * @param values values to read from; must be at least get_value_count() values
         */
        void set_values(const std::vector<Number> &values) noexcept;

        /**
         * Get the comment
         * @return the comment
         */
        const char *get_comment() const noexcept {
            return this->comment;
        }
        
        /**
         * Get the minimum value
         * @return minimum value or nullopt if there is no minimum
         */
        std::optional<Number> get_minimum() const noexcept {
            return this->minimum;
        }
        
        /**
         * Get the maximum value
         * @return maximum value or nullopt if there is no maximum
         */
        std::optional<Number> get_maximum() const noexcept {
            return this->maximum;
        }

        /**
         * Get the dependency
         * @return dependency
         */
        Dependency &get_dependency() noexcept {
            return *reinterpret_cast<Dependency *>(this->address);
        }

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

        /**
         * Get the string value
         * @return string value
         */
        const char *get_string() const noexcept {
            return reinterpret_cast<HEK::TagString *>(this->address)->string;
        }

        /**
         * Get the unit
         * @return unit
         */
        const char *get_unit() const noexcept {
            return this->unit;
        }

        /**
         * Set the string value
         * @param string string value
         */
        void set_string(const char *string) {
            auto &str_to_write = reinterpret_cast<HEK::TagString *>(this->address)->string;
            std::fill(str_to_write, str_to_write + sizeof(str_to_write), 0);
            std::strncpy(str_to_write, string, sizeof(str_to_write) - 1);
        }

        /**
         * Get the object in the array
         * @param  index index
         * @return       object in array
         */
        ParserStruct &get_object_in_array(std::size_t index) {
            return this->get_object_in_array_fn(index, this->address);
        }

        /**
         * Get the number of elements in the array
         * @return number of elements in array
         */
        std::size_t get_array_size() noexcept {
            return this->get_array_size_fn(this->address);
        }

        /**
         * Get the minimum number of elements in the array
         * @return minimum number of elements in array
         */
        std::size_t get_array_minimum_size() noexcept {
            return this->min_array_size;
        }

        /**
         * Get the maximum number of elements in the array
         * @return maximum number of elements in array
         */
        std::size_t get_array_maximum_size() noexcept {
            return this->max_array_size;
        }

        /**
         * Delete the objects in the array
         * @param index index of first object
         * @param count number of objects to delete
         */
        void delete_objects_in_array(std::size_t index, std::size_t count) {
            return this->delete_objects_in_array_fn(index, count, this->address);
        }

        /**
         * Insert new objects in the array
         * @param index index of first object to insert to
         * @param count number of objects to create
         */
        void insert_objects_in_array(std::size_t index, std::size_t count) {
            return this->insert_objects_in_array_fn(index, count, this->address);
        }

        /**
         * Insert new objects in the array
         * @param index_from index of first object to copy
         * @param index_to   index of first object to insert to
         * @param count      number of objects to create
         */
        void duplicate_objects_in_array(std::size_t index_from, std::size_t index_to, std::size_t count) {
            return this->duplicate_objects_in_array_fn(index_from, index_to, count, this->address);
        }

        /**
         * Get whether or not this is a bounds
         * @return is bounds
         */
        bool is_bounds() const noexcept {
            return this->bounds;
        }

        /**
         * Read the string
         * @return string
         */
        const char *read_string() const noexcept {
            return reinterpret_cast<HEK::TagString *>(this->address)->string;
        }

        /**
         * Write the string
         * @param string new string
         */
        void write_string(const char *string) const noexcept {
            std::strncpy(reinterpret_cast<HEK::TagString *>(this->address)->string, string, sizeof(HEK::TagString::string) - 1);
        }

        /**
         * Read the enum
         * @return enum
         */
        const char *read_enum() const {
            return this->read_enum_fn(address);
        }

        /**
         * Write the enum
         * @param value value to write
         */
        void write_enum(const char *value) {
            this->write_enum_fn(value, address);
        }

        /**
         * Read the bitfield value
         * @param  field field name
         * @return value
         */
        bool read_bitfield(const char *field) const {
            return this->read_bitfield_fn(field, address);
        }

        /**
         * Read the bitfield value
         * @param  field field name
         * @param  value value name
         */
        void write_bitfield(const char *field, bool value) {
            this->write_bitfield_fn(field, value, address);
        }

        /**
         * Read the data size
         * @return data size
         */
        std::size_t get_data_size() const noexcept {
            return reinterpret_cast<const std::vector<std::byte> *>(this->address)->size();
        }

        /**
         * List all enum values
         * @return all enum values
         */
        std::vector<const char *> list_enum() const noexcept {
            return this->list_enum_fn();
        }

        /**
         * List all enum values with definition values
         * @return all enum values
         */
        std::vector<const char *> list_enum_pretty() const noexcept {
            return this->list_enum_pretty_fn();
        }

        using get_object_in_array_fn_type = ParserStruct &(*)(std::size_t index, void *addr);
        using get_array_size_fn_type = std::size_t (*)(const void *addr);
        using delete_objects_in_array_fn_type = void (*)(std::size_t index, std::size_t count, void *addr);
        using insert_objects_in_array_fn_type = void (*)(std::size_t index, std::size_t count, void *addr);
        using duplicate_objects_in_array_fn_type = void (*)(std::size_t index_from, std::size_t index_to, std::size_t count, void *addr);

        /**
         * Get the object in the array (for get_object_in_array_fn)
         * @param  index index of object
         * @param  addr  address of object
         * @return       object in array
         */
        template <typename T>
        static ParserStruct &get_object_in_array_template(std::size_t index, void *addr) {
            auto &array = *reinterpret_cast<T *>(addr);
            auto size = array.size();
            if(index >= size) {
                eprintf_error("Index is out of bounds %zu >= %zu", index, size);
                throw OutOfBoundsException();
            }
            return array[index];
        }

        /**
         * Get the object in the array (for get_array_size_fn)
         * @param  addr  address of object
         * @return       size of array
         */
        template <typename T>
        static std::size_t get_array_size_template(const void *addr) {
            return reinterpret_cast<const T *>(addr)->size();
        }

        /**
         * Delete the object in the array (for delete_objects_in_array_fn)
         * @param  index index of first object to delete
         * @param  count count of objects to delete
         * @param  addr  address of object
         */
        template <typename T>
        static void delete_objects_in_array_template(std::size_t index, std::size_t count, void *addr) {
            auto &array = *reinterpret_cast<T *>(addr);
            assert_range_exists(index, count, array);
            array.erase(array.begin() + index, array.begin() + index + count);
        }

        /**
         * Get the object in the array (for insert_objects_in_array_fn)
         * @param  index index for objects to be inserted
         * @param  count count of objects to delete
         * @param  addr  address of object
         */
        template <typename T>
        static void insert_object_in_array_template(std::size_t index, std::size_t count, void *addr) {
            auto &array = *reinterpret_cast<T *>(addr);
            auto size = array.size();
            if(index > size) { // can be inclusive if we're adding objects to the very end
                eprintf_error("Index is out of bounds %zu > %zu", index, size);
                throw OutOfBoundsException();
            }
            array.insert(array.begin() + index, count, typename T::value_type());
        }

        /**
         * Duplicate the object in the array (for duplicate_objects_in_array_fn)
         * @param  index index for objects to be inserted
         * @param  count count of objects to delete
         * @param  addr  address of object
         */
        template <typename T>
        static void duplicate_object_in_array_template(std::size_t index_from, std::size_t index_to, std::size_t count, void *addr) {
            auto &array = *reinterpret_cast<T *>(addr);
            assert_range_exists(index_from, count, array);
            auto size = array.size();
            if(index_to > size) { // can be inclusive if we're adding objects to the very end
                eprintf_error("Index is out of bounds %zu > %zu", index_to, size);
                throw OutOfBoundsException();
            }
            array.insert(array.begin() + index_to, count, typename T::value_type());

            // Copy things over, handling overlap
            std::vector<std::size_t> copied_indices(count);
            for(std::size_t i = 0; i < count; i++) {
                std::size_t q = index_from + i;
                if(index_from + i >= index_to) {
                    q += count;
                }
                array[index_to + i] = array[q];
            }
        }

        using list_enum_fn_type = std::vector<const char *>(*)();

        using read_enum_fn_type = const char *(*)(void *address);
        using write_enum_fn_type = void (*)(const char *value, void *address);

        using read_bitfield_fn_type = bool (*)(const char *value, void *address);
        using write_bitfield_fn_type = void (*)(const char *value, bool flag, void *address);

        /**
         * Return a list of all of the possible enums
         * @return vector of all possible enums
         */
        template <typename T, const char *(*convert_fn)(T), std::size_t count, T *ignore_list = nullptr, std::size_t ignore_count = 0>
        static std::vector<const char *> list_enum_template() {
            std::vector<const char *> out;
            out.reserve(count - ignore_count);
            for(std::size_t i = 0; i < count; i++) {
                // Check if we need to ignore it
                bool ignore = false;
                for(std::size_t g = 0; (g + 1) < (ignore_count + 1); g++) { // adding 1 to both sides seems pointless, but it removes a warning when ignore_count is 0
                    if(ignore_list[g] == i) {
                        ignore = true;
                        break;
                    }
                }
                if(ignore) {
                    continue;
                }
                
                // Let's-a-go
                out.emplace_back(convert_fn(static_cast<T>(i)));
            }
            return out;
        }

        /**
         * Return a list of all of the possible enums
         * @return vector of all possible enums
         */
        template <typename T, const char *(*convert_fn)(T), std::size_t count, std::size_t mask>
        static std::vector<const char *> list_bitmask_template() {
            std::vector<const char *> out;
            out.reserve(count);
            for(std::size_t i = 0; i < count; i++) {
                auto bit = static_cast<std::size_t>(1) << i;
                if(!(bit & mask)) {
                    continue;
                }
                out.emplace_back(convert_fn(static_cast<T>(bit)));
            }
            return out;
        }

        /**
         * Read the enum value at the address
         * @param  address value to read
         * @return         value as string
         */
        template <typename T, const char *(*convert_fn)(T)>
        static const char *read_enum_template(void *address) {
            return convert_fn(*reinterpret_cast<T *>(address));
        }

        /**
         * Write the enum value at the address
         * @param  value   value to write
         * @param  address value to write to
         * @return         value as string
         */
        template <typename T, T(*convert_fn)(const char *)>
        static void write_enum_template(const char *value, void *address) {
            *reinterpret_cast<T *>(address) = convert_fn(value);
        }

        /**
         * Read the bitfield value at the address
         * @param  value   bit name to read
         * @param  address value to read from
         * @return         value
         */
        template <typename T, T(*convert_fn)(const char *)>
        static bool read_bitfield_template(const char *value, void *address) {
            return static_cast<T>(convert_fn(value)) & *reinterpret_cast<T *>(address);
        }

        /**
         * Write the bitfield value to the address
         * @param  value   bit name to read
         * @param  bool    bit to write
         * @param  address value to read from
         */
        template <typename T, T(*convert_fn)(const char *)>
        static void write_bitfield_template(const char *value, bool flag, void *address) {
            if(flag) {
                *reinterpret_cast<T *>(address) = static_cast<T>(*reinterpret_cast<T *>(address) | convert_fn(value));
            }
            else {
                *reinterpret_cast<T *>(address) = static_cast<T>(*reinterpret_cast<T *>(address) & ~convert_fn(value));
            }
        }

        /**
         * Get all of the allowed classes of the dependency
         * @return all allowed classes
         */
        const std::vector<TagClassInt> &get_allowed_classes() const noexcept {
            return this->allowed_classes;
        }

        /**
         * Get whether the value is read only or not
         * @return true if value is read only
         */
        bool is_read_only() const noexcept {
            return this->read_only;
        }

        /**
         * Instantiate a ParserStructValue with a group start
         * @param name    name of the group
         * @param comment comments
         */
        ParserStructValue(
            const char *name,
            const char *comment
        );

        /**
         * Instantiate a ParserStructValue with a dependency
         * @param name            name of the dependency
         * @param member_name     variable name of the dependency
         * @param comment         comments
         * @param dependency      pointer to the dependency
         * @param allowed_classes array of allowed classes
         * @param count           number of allowed classes in array
         * @param read_only       value is read only
         */
        ParserStructValue(
            const char *       name,
            const char *       member_name,
            const char *       comment,
            Dependency *       dependency,
            const TagClassInt *allowed_classes,
            std::size_t        count,
            bool               read_only
        );

        /**
         * Instantiate a ParserStructValue with an array
         * @param name                          name of the array
         * @param member_name                   variable name of the array
         * @param comment                       comments
         * @param array                         pointer to the array
         * @param get_object_in_array_fn        pointer to function for getting object in array
         * @param get_array_size_fn             pointer to function for getting the size of array
         * @param delete_objects_in_array_fn    pointer to function for deleting objects from an array
         * @param insert_objects_in_array_fn    pointer to function for inserting objects in an array
         * @param duplicate_objects_in_array_fn pointer to function for duplicating objects in an array
         * @param minimum_array_size            minimum number of elements in the array
         * @param maximum_array_size            maximum number of elements in the array
         * @param read_only                     value is read only
         */
        ParserStructValue(
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
        );

        /**
         * Instantiate a ParserStructValue with a TagString
         * @param name        name of the value
         * @param member_name variable name of the value
         * @param comment     comments
         * @param string      pointer to string
         * @param read_only   value is read only
         */
        ParserStructValue(
            const char *    name,
            const char *    member_name,
            const char *    comment,
            HEK::TagString *string,
            bool            read_only
        );

        /**
         * Instantiate a ParserStructValue with a TagDataOffset
         * @param name        name of the value
         * @param member_name variable name of the value
         * @param comment     comments
         * @param string      pointer to string
         * @param read_only   value is read only
         */
        ParserStructValue(
            const char *            name,
            const char *            member_name,
            const char *            comment,
            std::vector<std::byte> *offset,
            bool                    read_only
        );

        /**
         * Instantiate a ParserStructValue with a TagEnum
         * @param name                 name of the value
         * @param member_name          variable name of the value
         * @param comment              comments
         * @param value                pointer to value
         * @param list_enum_fn         pointer to function for listing enums
         * @param list_enum_pretty_fn  pointer to function for listing enums with definition naming
         * @param read_enum_fn         pointer to function for reading enums
         * @param write_enum_fn        pointer to function for writing enums
         * @param read_only            value is read only
         */
        ParserStructValue(
            const char *       name,
            const char *       member_name,
            const char *       comment,
            void *             value,
            list_enum_fn_type  list_enum_fn,
            list_enum_fn_type  list_enum_pretty_fn,
            read_enum_fn_type  read_enum_fn,
            write_enum_fn_type write_enum_fn,
            bool               read_only
        );

        /**
         * Instantiate a ParserStructValue with a bitfield
         * @param name                 name of the value
         * @param member_name          variable name of the value
         * @param comment              comments
         * @param value                pointer to value
         * @param list_enum_fn         pointer to function for listing enums
         * @param list_enum_pretty_fn  pointer to function for listing enums with definition naming
         * @param read_bitfield_fn     pointer to function for reading enums
         * @param write_bitfield_fn    pointer to function for writing enums
         * @param read_only            value is read only
         */
        ParserStructValue(
            const char *           name,
            const char *           member_name,
            const char *           comment,
            void *                 value,
            list_enum_fn_type      list_enum_fn,
            list_enum_fn_type      list_enum_pretty_fn,
            read_bitfield_fn_type  read_bitfield_fn,
            write_bitfield_fn_type write_bitfield_fn,
            bool                   read_only
        );

        /**
         * Instantiate a ParserStructValue with a value
         * @param name        name of the value
         * @param member_name variable name of the value
         * @param comment     comments
         * @param object      pointer to the object
         * @param type        type of value
         * @param unit        unit to use
         * @param count       number of values (if multiple values or bounds)
         * @param bounds      whether or not this is bounds
         * @param read_only   value is read only
         * @param minimum     optional minimum value
         * @param maximum     optional maximum value
         */
        ParserStructValue(
            const char *          name,
            const char *          member_name,
            const char *          comment,
            void *                object,
            ValueType             type,
            const char *          unit = nullptr,
            std::size_t           count = 1,
            bool                  bounds = false,
            bool                  read_only = false,
            std::optional<Number> minimum = std::nullopt,
            std::optional<Number> maximum = std::nullopt
        );

    private:
        const char *name;
        const char *member_name;
        const char *comment;
        ValueType type;
        void *address;
        std::vector<TagClassInt> allowed_classes;
        std::size_t count = 1;
        bool bounds = false;
        const char *unit = nullptr;
        std::optional<Number> minimum;
        std::optional<Number> maximum;

        get_object_in_array_fn_type get_object_in_array_fn = nullptr;
        get_array_size_fn_type get_array_size_fn = nullptr;
        delete_objects_in_array_fn_type delete_objects_in_array_fn = nullptr;
        insert_objects_in_array_fn_type insert_objects_in_array_fn = nullptr;
        duplicate_objects_in_array_fn_type duplicate_objects_in_array_fn = nullptr;

        list_enum_fn_type list_enum_fn = nullptr;
        list_enum_fn_type list_enum_pretty_fn = nullptr;
        read_enum_fn_type read_enum_fn = nullptr;
        write_enum_fn_type write_enum_fn = nullptr;
        read_bitfield_fn_type read_bitfield_fn = nullptr;
        write_bitfield_fn_type write_bitfield_fn = nullptr;

        std::size_t min_array_size;
        std::size_t max_array_size;

        bool read_only = false;

        template <typename T>
        static void assert_range_exists(std::size_t index, std::size_t count, const T &array) {
            if(count == 0) {
                return;
            }

            std::size_t size = array.size();
            if(count > size || index >= size || (index + count) > size) {
                eprintf_error("Range is out of bounds (%zu + %zu) > %zu", index, count, size);
                throw OutOfBoundsException();
            }
        }
    };

    struct ParserStruct {
        /**
         * Get whether or not the data is formatted for cache files.
         * @return true if data is formatted for cache files
         */
        bool is_cache_formatted() const noexcept { return this->cache_formatted; };

        /**
         * Parse the HEK tag file
         * @param  data        Tag file data to read from
         * @param  data_size   Size of the tag file
         * @param  postprocess Do post-processing on data, such as default values
         * @return             parsed tag data
         */
        static std::unique_ptr<ParserStruct> parse_hek_tag_file(const std::byte *data, std::size_t data_size, bool postprocess = false);

        /**
         * Generate a tag base struct
         * @param  tag_class tag class
         * @return           a tag reference
         */
        static std::unique_ptr<ParserStruct> generate_base_struct(TagClassInt tag_class);

        /**
         * Get a vector of all tag classes
         * @param  exclude_subclasses exclude all subclasses
         * @return all tag classes
         */
        static std::vector<TagClassInt> all_tag_classes(bool exclude_subclasses);

        /**
         * Check for broken enums
         * @param  reset_enums attempt to fix the enums by setting them to 0
         * @return             true if broken enums were found; false if not
         */
        virtual bool check_for_broken_enums(bool reset_enums) = 0;

        /**
         * Check for broken indices
         * @param  null_indices attempt to fix the enums by setting them to a null index (65535)
         * @return              true if broken indices were found; false if not
         */
        virtual bool check_for_invalid_indices(bool null_indices) = 0;

        /**
         * Check for broken indices
         * @param  null_indices attempt to fix the enums by setting them to a null index (65535)
         * @param  stack        stack to check
         * @return              true if broken indices were found; false if not
         */
        virtual bool check_for_invalid_indices(bool null_indices, std::deque<std::tuple<const ParserStruct *, std::size_t, const char *>> &stack) = 0;

        /**
         * Check for invalid references
         * @param  null_references attempt to fix the references by nulling them out
         * @return                 true if invalid references were found; false if not
         */
        virtual bool check_for_invalid_references(bool null_references) = 0;

        /**
         * Check for nonnormal vectors
         * @param  normalize normalize vectors if they aren't normalized
         * @return           true if nonnormal vectors were found
         */
        virtual bool check_for_nonnormal_vectors(bool normalize) = 0;
        
        /**
         * Check for invalid ranges
         * @param clamp attempt to fix the ranges by clamping them
         * @return      true if invalid ranges were found; false if not
         */
        virtual bool check_for_invalid_ranges(bool clamp) = 0;

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
         * @param stack        stack of structs to use
         */
        virtual void compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::optional<std::size_t> bsp = std::nullopt, std::size_t offset = 0, std::deque<const ParserStruct *> *stack = nullptr) = 0;

        /**
         * Convert the struct into HEK tag data to be built into a cache file.
         * @param  generate_header_class generate a cache file header with the class, too
         * @param  clear_on_save         clear data as it's being saved (reduces memory usage but you can't work on the tag anymore)
         * @return cache file data
         */
        virtual std::vector<std::byte> generate_hek_tag_data(std::optional<TagClassInt> generate_header_class = std::nullopt, bool clear_on_save = false) = 0;

        /**
         * Refactor the tag reference, replacing all references with the given reference. Paths must use Halo path separators.
         * @param from_path  Path to look for
         * @param from_class Class to look for
         * @param to_path    Path to replace with
         * @param to_class   Class to replace with
         * @return           number of references replaced
         */
        virtual std::size_t refactor_reference(const char *from_path, TagClassInt from_class, const char *to_path, TagClassInt to_class) = 0;

        /**
         * Refactor the tag reference, replacing all references with the given reference. Paths must use Halo path separators.
         * @param from  Path to look for
         * @param to    Path to replace with
         * @return      number of references replaced
         */
        std::size_t refactor_reference(const File::TagFilePath &from, const File::TagFilePath &to);

        /**
         * Refactor the tag references, replacing all references with the given reference. Paths must use Halo path separators.
         * @param changes Changes to make (first is from, second is to)
         * @return        number of references replaced
         */
        std::size_t refactor_references(const std::vector<std::pair<File::TagFilePath, File::TagFilePath>> &replacements);

        /**
         * Get the values in the struct
         * @return values in the struct
         */
        virtual std::vector<ParserStructValue> get_values() = 0;

        /**
         * Get whether or not the struct has a title
         * @return true if struct has title
         */
        virtual bool has_title() const;

        /**
         * Get the title of the struct
         * @return title of the struct
         */
        virtual const char *title() const;

        /**
         * Get the name of the struct
         * @return name of the struct
         */
        virtual const char *struct_name() const = 0;
        
        /**
         * Compare the struct against another struct
         * @param what            struct to compare against
         * @param precision       allow small differences for floats (can account for minor precision differences but may slightly increase false positives)
         * @param ignore_volatile ignore data that can be added or removed when a map is compiled
         */
        virtual bool compare(const ParserStruct *what, bool precision = false, bool ignore_volatile = false) const = 0;
        
        bool operator==(const ParserStruct &other) const {
            return this->compare(&other);
        }
        
        bool operator!=(const ParserStruct &other) const {
            return !this->compare(&other);
        }

        virtual ~ParserStruct() = default;
    protected:
        bool cache_formatted = false;
    };
}

#endif
