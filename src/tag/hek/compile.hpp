/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__COMPILE_HPP
#define INVADER__TAG__HEK__COMPILE_HPP

#include <cstring>

#include "../../eprintf.hpp"
#include "../../hek/data_type.hpp"
#include "../../error.hpp"
#include "../compiled_tag.hpp"
#include "header.hpp"

namespace Invader {
    /**
     * Assert that the variable, size, is greater than or equal to compare_size
     * @param  compare_size size to check
     */
    #define ASSERT_SIZE(compare_size) if(size < static_cast<std::size_t>(compare_size)) { \
        eprintf("ASSERT_SIZE failed: %zu < " # compare_size " (%zu)\n", static_cast<std::size_t>(size), static_cast<std::size_t>(compare_size)); \
        throw OutOfBoundsException(); \
    }

    /**
     * Assert sizeof(type<BigEndian>) as well as reserve size bytes in the compiled variable.
     * @param  type type of struct to compile
     */
    #define RESERVE_DATA_COMPILE(type) ASSERT_SIZE(sizeof(type <BigEndian>)) \
                                       compiled.data.reserve(size + size/2);

    /**
     * Add amt bytes to data and subtract amt from size
     * @param  amt value to add/subtract
     */
    #define INCREMENT_DATA_PTR(amt) data += amt; size -= amt;

    /**
     * Set what to default_value if what is equal to 0
     * @param  what          value to check/set
     * @param  default_value default value to use
     */
    #define DEFAULT_VALUE(what,default_value) if(what == 0) { what = default_value; }

    /**
     * Set what to default_value if what is non positive
     * @param  what          value to check/set
     * @param  default_value default value to use
     */
    #define DEFAULT_VALUE_POSITIVE(what,default_value) if(what <= 0) { what = default_value; }

    /**
     * Pad compiled.data until it's 32-bit
     */
    #define PAD_32_BIT compiled.data.insert(compiled.data.end(), REQUIRED_PADDING_32_BIT(compiled.data.size()), std::byte());

    /**
     * Begin adding reflexive. End with ADD_REFLEXIV_END
     * @param  reflexive_struct reflexive to add
     */
    #define ADD_REFLEXIVE_START(reflexive_struct) \
        {\
            const char *struct_name = # reflexive_struct; \
            try { \
                reflexive_struct .pointer = 0; \
                reflexive_struct .unknown = 0; \
                if(reflexive_struct .count > 0) { \
                    using ref_type = decltype(reflexive_struct)::struct_type_little; \
                    auto count = reflexive_struct .count.read(); \
                    auto ref_size = sizeof(ref_type) * count; \
                    auto first_struct_offset = compiled.data.size(); \
                    ASSERT_SIZE(ref_size); \
                    std::uint8_t padding = REQUIRED_PADDING_32_BIT(ref_size); \
                    if(skip_data) { \
                        reflexive_struct.count = 0;\
                    }\
                    else {\
                        ADD_POINTER_FROM_INT32(reflexive_struct .pointer.value, first_struct_offset) \
                        compiled.data.insert(compiled.data.end(), ref_size + padding, std::byte()); \
                    }\
                    auto *reflexives_big = reinterpret_cast<const decltype(reflexive_struct)::struct_type_big *>(data); \
                    INCREMENT_DATA_PTR(ref_size); \
                    for(std::size_t i = 0; i < count; i++) { \
                        ref_type reflexive = reflexives_big[i]; \
                        try {\
                            auto current_struct_offset = first_struct_offset + i * sizeof(reflexive);\
                            auto current_struct_address = reinterpret_cast<std::uintptr_t>(&reflexive);

        /**
         * Finish adding reflexive and copy reflexive data to compiled.data
         */
        #define ADD_REFLEXIVE_END \
                            if(!skip_data) { \
                                std::copy(reinterpret_cast<std::byte *>(current_struct_address), reinterpret_cast<std::byte *>(current_struct_address + sizeof(reflexive)), compiled.data.data() + current_struct_offset); \
                            } \
                        }\
                        catch(std::exception &) {\
                            eprintf("error adding reflexive #%zu for %s\n", i, struct_name); \
                            throw; \
                        }\
                    }\
                }\
            }\
            catch(std::exception &) { \
                eprintf("error adding reflexive for %s\n", struct_name); \
                throw; \
            }\
        }

    /**
     * Begin compiling a tag
     * @param  type main struct of tag
     */
    #define BEGIN_COMPILE(type) \
        RESERVE_DATA_COMPILE(type) \
        compiled.data.insert(compiled.data.end(), sizeof(type<BigEndian>), std::byte());\
        type <LittleEndian> tag = {};\
        bool skip_data = false;\
        std::size_t current_struct_offset = compiled.data.size() - sizeof(type<BigEndian>);\
        std::uintptr_t current_struct_address = reinterpret_cast<std::uintptr_t>(&tag);\
        tag = *reinterpret_cast<const type <BigEndian> *>(data);\
        if(!skip_data) {\
            INCREMENT_DATA_PTR(sizeof(tag));\
        }

    /**
     * Finish compiling tag and copy tag struct data to compiled.data. Also shrink asset_data and data to fit to reduce any unnecessary memory usage.
     */
    #define FINISH_COMPILE_COPY std::copy(reinterpret_cast<std::byte *>(current_struct_address), reinterpret_cast<std::byte *>(current_struct_address + sizeof(tag)), compiled.data.data() + current_struct_offset); \
                                compiled.data.shrink_to_fit(); \
                                compiled.asset_data.shrink_to_fit();

    /**
     * Finish compiling tag and copy tag struct data to compiled.data, then ensure there is no more data left to read
     * @throws throw an exception if there is extra tag data
     */
    #define FINISH_COMPILE \
        FINISH_COMPILE_COPY \
        if(size != 0) { \
            eprintf("unexpected extra %zu bytes\n", size); \
            throw ExtraTagDataException();\
        }

    /**
     * Add a dependency from a reflexive and copy all of the other reflexive's data
     * @param  reflexive_struct reflexive to add
     * @param  dependency       dependency member variable name
     */
    #define ADD_BASIC_DEPENDENCY_REFLEXIVE(reflexive_struct, dependency) \
        ADD_REFLEXIVE_START(reflexive_struct) \
        ADD_DEPENDENCY_ADJUST_SIZES(reflexive.dependency); \
        ADD_REFLEXIVE_END

    /**
     * Add a reflexive, copying all of its data
     * @param  reflexive_struct reflexive to add
     */
    #define ADD_REFLEXIVE(reflexive_struct) ADD_REFLEXIVE_START(reflexive_struct) ADD_REFLEXIVE_END

    /**
     * Add a dependency, then add and subtract the length of the dependency tag path to and from data and size, respectively
     * @param  dependency dependency to add
     * @throws throw an exception if the dependency is not valid
     */
    #define ADD_DEPENDENCY_ADJUST_SIZES(dependency) {\
        try { \
            auto amt = add_dependency(compiled, dependency, reinterpret_cast<std::uintptr_t>(&dependency) - current_struct_address + current_struct_offset, data, size, skip_data); \
            INCREMENT_DATA_PTR(amt); \
        } \
        catch(std::exception &) { \
            eprintf("error adding dependency " # dependency "\n"); \
            throw; \
        } \
    }

    /**
     * Add the pointer to compiled.pointer
     * @param  int32    pointer
     * @param  point_to offset in data that pointer points to
     */
    #define ADD_POINTER_FROM_INT32(int32, point_to) add_pointer(compiled, reinterpret_cast<std::uintptr_t>(&int32) - current_struct_address + current_struct_offset, point_to);

    /**
     * Add a dependency to a compiled tag
     * @param  compiled      reference to the compiled tag
     * @param  dependency    reference to the dependency from within the compiled tag
     * @param  offset        offset of the dependency
     * @param  path          pointer to the data where the path is
     * @param  max_path_size maximum size the path can be
     * @param  skip_data     whether or not to skip the data rather than actually add a dependency
     * @param  name          name of dependency (used for errors)
     * @return               size of path, including null terminator or 0 if no dependency
     */
    std::size_t add_dependency(CompiledTag &compiled, HEK::TagDependency<HEK::LittleEndian> &dependency, std::size_t offset, const std::byte *path, std::size_t max_path_size, bool skip_data);

    /**
     * Add a pointer to a compiled tag.
     * @param compiled reference to the compiled tag
     * @param pointer  offset to the pointer
     * @param point_to offset to where the pointer is pointing to
     */
    void add_pointer(CompiledTag &compiled, std::size_t offset, std::size_t point_to);
}
#endif
