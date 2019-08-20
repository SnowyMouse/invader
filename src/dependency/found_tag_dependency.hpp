/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#ifndef INVADER__DEPENDENCY_FOUND_TAG_DEPENDENCY_HPP
#define INVADER__DEPENDENCY_FOUND_TAG_DEPENDENCY_HPP

#include <string>
#include <vector>
#include "../hek/class_int.hpp"

namespace Invader {
    struct FoundTagDependency {
        std::string path;
        Invader::HEK::TagClassInt class_int;
        bool broken;
        std::string file_path;

        static std::vector<FoundTagDependency> find_dependencies(const char *tag_path_to_find, Invader::HEK::TagClassInt tag_int_to_find, std::vector<std::string> tags, bool reverse, bool recursive, bool &success);

        FoundTagDependency(std::string path, Invader::HEK::TagClassInt class_int, bool broken, std::string file_path) : path(path), class_int(class_int), broken(broken), file_path(file_path) {}
    };
}

#endif
