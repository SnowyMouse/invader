// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__FILE__FILE_HPP
#define INVADER__FILE__FILE_HPP

#include <vector>
#include <cstdlib>
#include <optional>

namespace Invader::File {
    /**
     * Attempt to open the file and read it all into a buffer
     * @param path path to the file
     * @return     a buffer holding the file or std::nullopt if failed
     */
    std::optional<std::vector<std::byte>> open_file(const char *path);

    /**
     * Attempt to save the file
     * @param  path path to the file
     * @param  data data to write
     * @return      true on success; false on failure
     */
    bool save_file(const char *path, const std::vector<std::byte> &data);

    /**
     * Convert a tag path to a file path
     * @param  tag_path   tag path to use
     * @param  tags       tags directory to use
     * @param  must_exist ensure the path exists
     * @return            file path to use if exists, or std::nullopt on failure
     */
    std::optional<std::string> tag_path_to_file_path(const std::string &tag_path, const std::vector<std::string> &tags, bool must_exist);

    /**
     * Convert a file path to a tag path
     * @param  tag_path   tag path to use
     * @param  tags       tags directory to use
     * @param  must_exist ensure the path exists
     * @return            tag path to use if exists, or std::nullopt on failure
     */
    std::optional<std::string> file_path_to_tag_path(const std::string &file_path, const std::vector<std::string> &tags, bool must_exist);
}

#endif
