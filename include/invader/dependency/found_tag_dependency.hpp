// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__DEPENDENCY__FOUND_TAG_DEPENDENCY_HPP
#define INVADER__DEPENDENCY__FOUND_TAG_DEPENDENCY_HPP

#include <string>
#include <filesystem>
#include <vector>
#include <optional>
#include "../hek/fourcc.hpp"

namespace Invader {
    struct FoundTagDependency {
        std::string path;
        Parser::TagFourCC fourcc;
        bool broken;
        std::optional<std::filesystem::path> file_path;

        static std::vector<FoundTagDependency> find_dependencies(const char *tag_path_to_find, Parser::TagFourCC tag_int_to_find, std::vector<std::filesystem::path> tags, bool reverse, bool recursive, bool &success);

        FoundTagDependency(std::string path, Parser::TagFourCC fourcc, bool broken, std::optional<std::filesystem::path> file_path) : path(path), fourcc(fourcc), broken(broken), file_path(file_path) {}
    };
}

#endif
