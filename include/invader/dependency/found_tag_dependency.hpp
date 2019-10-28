// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__DEPENDENCY__FOUND_TAG_DEPENDENCY_HPP
#define INVADER__DEPENDENCY__FOUND_TAG_DEPENDENCY_HPP

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
