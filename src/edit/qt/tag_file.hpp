// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_FILE_HPP
#define INVADER__EDIT__QT__TAG_FILE_HPP

#include <filesystem>
#include <string>
#include <vector>
#include <invader/hek/class_int.hpp>

namespace Invader::EditQt {
    struct TagFile {
        /** Full filesystem path */
        std::filesystem::path full_path;

        /** Tag path Halo uses */
        std::string tag_path;

        /** Tag path elements (for directory browsing) */
        std::vector<std::string> tag_path_separated;

        /** Tag directory this tag uses (lower number = higher priority) */
        std::size_t tag_directory = {};

        /** Tag class of this tag */
        HEK::TagClassInt tag_class_int = {};

        /** The tag is marked as ignored */
        bool ignored = false;
    };
}

#endif
