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
    struct ParserStruct;

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
        static std::size_t get_array_size_template(void *addr) {
            return reinterpret_cast<T *>(addr)->size();
        }

        /**
         * Delete the object in the array (for delete_objects_in_array_fn)
         * @param  index index of first object to delete
         * @param  count count of objects to delete
         * @param  addr  address of object
         */
        template <typename T>
        static void delete_objects_in_array_template(std::size_t index, std::size_t count, void *addr) {
            auto *array = reinterpret_cast<T *>(addr);
            assert_range_exists(index, count, array);
            array->erase(array->begin() + index, array->begin() + index + count);
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
                eprintf_error("Index is out of bounds %zu > %zu", index, size);
                throw OutOfBoundsException();
            }
            array.insert(array.begin() + index_from, count, typename T::value_type());

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

        /**
         * Instantiate a ParserStructValue
         * @param type                          type of value
         * @param object                        pointer to the object
         * @param get_object_in_array_fn        pointer to function for getting object in array (if it's an array)
         * @param get_array_size_fn             pointer to function for getting the size of array (if it's an array)
         * @param delete_objects_in_array_fn    pointer to function for deleting objects from an array (if it's an array)
         * @param insert_objects_in_array_fn    pointer to function for inserting objects in an array (if it's an array)
         * @param duplicate_objects_in_array_fn pointer to function for duplicating objects in an array (if it's an array)
         */
        ParserStructValue(
            ValueType                          type,
            void *                             object,
            get_object_in_array_fn_type        get_object_in_array_fn = nullptr,
            get_array_size_fn_type             get_array_size_fn = nullptr,
            delete_objects_in_array_fn_type    delete_objects_in_array_fn = nullptr,
            insert_objects_in_array_fn_type    insert_objects_in_array_fn = nullptr,
            duplicate_objects_in_array_fn_type duplicate_objects_in_array_fn = nullptr
        );

    private:
        ValueType type;
        const char *name;
        void *address;

        get_object_in_array_fn_type get_object_in_array_fn;
        get_array_size_fn_type get_array_size_fn;
        delete_objects_in_array_fn_type delete_objects_in_array_fn;
        insert_objects_in_array_fn_type insert_objects_in_array_fn;
        duplicate_objects_in_array_fn_type duplicate_objects_in_array_fn;

        template <typename T>
        static void assert_range_exists(std::size_t index, std::size_t count, T *array) {
            if(count == 0) {
                return;
            }

            std::size_t size = array->size();
            if(count >= size || index >= size || (index + count) > size) {
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
         * Refactor the tag reference, replacing all references with the given reference. Paths must use Halo path separators.
         * @param from_path  Path to look for
         * @param from_class Class to look for
         * @param to_path    Path to replace with
         * @param to_class   Class to replace with
         * @return           number of references replaced
         */
        virtual std::size_t refactor_reference(const char *from_path, TagClassInt from_class, const char *to_path, TagClassInt to_class) = 0;

        /**
         * Get the values in the struct
         * @return values in the struct
         */
        virtual std::vector<ParserStructValue> get_values() = 0;

        virtual ~ParserStruct() = default;
    protected:
        bool cache_formatted = false;
    };
}

#endif
