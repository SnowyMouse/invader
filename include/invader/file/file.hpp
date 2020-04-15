// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__FILE__FILE_HPP
#define INVADER__FILE__FILE_HPP

#include <vector>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <mutex>

#include "../hek/class_int.hpp"

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
     * @param  tag_path   tag file path to use
     * @param  tags       tags directory to use
     * @param  must_exist ensure the path exists
     * @return            tag path to use if exists, or std::nullopt on failure
     */
    std::optional<std::string> file_path_to_tag_path(const std::string &file_path, const std::vector<std::string> &tags, bool must_exist);

    /**
     * Attempt to resolve a file path to a tag path using an extension
     * @param tag_path           tag path to use
     * @param tags               tags directory to use
     * @param expected_extension extension to use and remove from the path, if found
     */
    std::optional<std::string> file_path_to_tag_path_with_extension(const std::string &tag_path, const std::vector<std::string> &tags, const std::string &expected_extension);

    /**
     * File path holder
     */
    struct TagFilePath {
        /** Path without extension */
        std::string path;

        /** Class of the tag */
        TagClassInt class_int;
        
        /** Join the path and class into one path */
        std::string join() {
            return path + "." + HEK::tag_class_to_extension(class_int);
        }
        
        TagFilePath() = default;
        TagFilePath(const TagFilePath &copy) = default;
        TagFilePath(const std::string &path, TagClassInt class_int) : path(path), class_int(class_int) {}
        bool operator==(const TagFilePath &other) const noexcept {
            return other.class_int == this->class_int && other.path == this->path;
        }
        bool operator!=(const TagFilePath &other) const noexcept {
            return other.class_int != this->class_int || other.path != this->path;
        }
    };

    /**
     * Attempt to split the tag class extension from a path
     * @param tag_path path to split
     * @return         path followed by extension or nullptr if failed
     */
    std::optional<TagFilePath> split_tag_class_extension(const std::string &tag_path);

    /**
     * Attempt to split the tag class extension from a path
     * @param tag_path path to split
     * @return         path followed by extension or nullptr if failed
     */
    std::optional<TagFilePath> split_tag_class_extension_chars(const char *tag_path);

    struct TagFile {
        /** Full filesystem path */
        std::filesystem::path full_path;

        /** Virtual tag path */
        std::string tag_path;

        /** Tag directory this tag uses (lower number = higher priority) */
        std::size_t tag_directory = {};

        /** Tag class of this tag */
        HEK::TagClassInt tag_class_int = {};

        /**
         * Split the tag path
         */
        std::vector<std::string> split_tag_path();
    };

    /**
     * Read a tags directory
     * @param  tags   tag directories
     * @param  status optional pointer to a size_t to store the current number of tags loaded (for status bars)
     * @return        all tags in the folder
     */
    std::vector<TagFile> load_virtual_tag_folder(const std::vector<std::string> &tags, std::pair<std::mutex, std::size_t> *status = nullptr);

    /**
     * Convert the tag path to a path using the system's preferred separators
     * @param  tag_path tag path input
     */
    void halo_path_to_preferred_path_chars(char *tag_path) noexcept;

    /**
     * Convert the tag path to a path using the Halo's separators (backslashes)
     * @param  tag_path tag path input
     */
    void preferred_path_to_halo_path_chars(char *tag_path) noexcept;

    /**
     * Convert the tag path to a path using the system's preferred separators
     * @param  tag_path tag path input
     * @return          output path
     */
    std::string halo_path_to_preferred_path(const std::string &tag_path);

    /**
     * Convert the tag path to a path using the Halo's separators (backslashes)
     * @param  tag_path tag path input
     * @return          output path
     */
    std::string preferred_path_to_halo_path(const std::string &tag_path);

    /**
     * Remove trailing slashes from the path
     * @param  path path to remove trailing slashes from
     * @return      output path
     */
    std::string remove_trailing_slashes(const std::string &path);

    /**
     * Remove trailing slashes from the path
     * @param  path path to remove trailing slashes from
     */
    void remove_trailing_slashes_chars(char *path);

    /**
     * Remove duplicate slashes from the path
     * @param  path path to remove duplicate slashes from
     * @return      path with removed duplicate slashes
     */
    std::string remove_duplicate_slashes(const std::string &path);

    /**
     * Remove duplicate slashes from the path
     * @param  path path to remove duplicate slashes from
     */
    void remove_duplicate_slashes_chars(char *path);

    /**
     * Get the base name of the tag path
     * @param  tag_path tag path to get
     * @return          base name
     */
    const char *base_name_chars(const char *tag_path) noexcept;

    /**
     * Get the base name of the tag path
     * @param  tag_path tag path to get
     * @return          base name
     */
    char *base_name_chars(char *tag_path) noexcept;

    /**
     * Get the base name of the tag path
     * @param  tag_path       tag path to get
     * @param  drop_extension drop the extension
     * @return                base name
     */
    std::string base_name(const std::string &tag_path, bool drop_extension = false);
    
    /**
     * Check working directory (April Fools)
     * @param file file to look for
     */
    void check_working_directory(const char *file);
}

#endif
