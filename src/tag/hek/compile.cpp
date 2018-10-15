/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include <iostream>

#include "compile.hpp"

namespace Invader {
    std::size_t add_dependency(CompiledTag &compiled, HEK::TagDependency<HEK::LittleEndian> &dependency, std::size_t offset, const std::byte *path, std::size_t max_path_size, bool skip_data, const char *name) {
        std::size_t path_size = dependency.path_size;

        // Zero stuff out
        dependency.path_pointer = 0;
        dependency.path_size = 0;

        // If the size > 0, there's a dependency
        if(path_size > 0) {
            // Max size is too small or path is not null terminated?
            if(max_path_size <= path_size || reinterpret_cast<const unsigned char *>(path)[path_size] != 0x00) {
                if(name) {
                    #ifndef NO_OUTPUT
                    std::cerr << "Could not parse dependency for " << name << "\n";
                    #endif
                }
                throw InvalidDependencyException();
            }

            // Add our dependency if we aren't skipping
            if(skip_data) {
                dependency.tag_class_int = HEK::TagClassInt::TAG_CLASS_NONE;
                *reinterpret_cast<std::uint32_t *>(dependency.tag_id.value) = 0xFFFFFFFF;
            }
            else {
                compiled.dependencies.push_back(CompiledTagDependency { offset, reinterpret_cast<const char *>(path), dependency.tag_class_int });
            }

            // Set the return value to the size of the path, including the null byte
            return path_size + 1;
        }
        else {
            *reinterpret_cast<std::uint32_t *>(dependency.tag_id.value) = 0xFFFFFFFF;
            return 0;
        }
    }

    void add_pointer(CompiledTag &compiled, std::size_t offset, std::size_t point_to) {
        compiled.pointers.push_back(CompiledTagPointer { offset, point_to });
    }
}
