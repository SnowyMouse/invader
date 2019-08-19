/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include "compile.hpp"

namespace Invader {
    std::size_t add_dependency(CompiledTag &compiled, HEK::TagDependency<HEK::LittleEndian> &dependency, std::size_t offset, const std::byte *path, std::size_t max_path_size, bool skip_data) {
        std::size_t path_size = dependency.path_size;

        // Zero stuff out
        dependency.path_pointer = 0;
        dependency.path_size = 0;

        // If the size > 0, there's a dependency
        if(path_size > 0) {
            // Max size is too small
            if(max_path_size <= path_size) {
                eprintf("path length exceeds the remaining tag data length\n");
                throw InvalidDependencyException();
            }

            // Path is not null terminated
            if(reinterpret_cast<const unsigned char *>(path)[path_size] != 0x00) {
                eprintf("path is not null-terminated or is too long\n");
                throw InvalidDependencyException();
            }

            // Add our dependency if we aren't skipping
            if(skip_data) {
                dependency.tag_class_int = HEK::TagClassInt::TAG_CLASS_NONE;
                dependency.tag_id = HEK::TagID::null_tag_id();
            }
            else {
                std::string path_str(reinterpret_cast<const char *>(path));

                // Make sure the length is correct
                if(path_str.length() != path_size) {
                    eprintf("path length is wrong (expected %zu; got %zu)\n", path_size, path_str.length());
                    throw InvalidDependencyException();
                }
                compiled.dependencies.push_back(CompiledTagDependency { offset, path_str, dependency.tag_class_int, false });
            }

            // Set the return value to the size of the path, including the null byte
            return path_size + 1;
        }
        else {
            dependency.tag_id = HEK::TagID::null_tag_id();
            return 0;
        }
    }

    void add_pointer(CompiledTag &compiled, std::size_t offset, std::size_t point_to) {
        compiled.pointers.push_back(CompiledTagPointer { offset, point_to });
    }
}
