// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__FILE__FILE_HPP
#define INVADER__FILE__FILE_HPP

#include <vector>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <mutex>

#include "../hek/fourcc.hpp"

namespace Invader::File {
    #ifdef INVADER_FORCE_PORTABLE_PREFERRED_PATHS
    #define INVADER_PREFERRED_PATH_SEPARATOR '/'
    #else
    #define INVADER_PREFERRED_PATH_SEPARATOR std::filesystem::path::preferred_separator
    #endif

    /**
     * File path holder
     */
    struct TagFilePath {
        /** Path without extension */
        std::string path;

        /** Class of the tag */
        TagFourCC fourcc;
        
        /** Join the path and class into one path */
        std::string join() const {
            return path + "." + HEK::tag_fourcc_to_extension(fourcc);
        }
        
        TagFilePath() = default;
        TagFilePath(const TagFilePath &copy) = default;
        TagFilePath(const std::string &path, TagFourCC fourcc) : path(path), fourcc(fourcc) {}
        TagFilePath &operator=(const TagFilePath &) = default;
        
        std::strong_ordering operator<=>(const TagFilePath &other) const noexcept {
            auto space_ship = this->path <=> other.path;
            if(space_ship == 0) {
                return this->fourcc <=> other.fourcc;
            }
            else {
                return space_ship;
            }
        }
        bool operator==(const TagFilePath &other) const noexcept {
            return (*this <=> other) == std::strong_ordering::equivalent;
        }
        bool operator!=(const TagFilePath &other) const noexcept {
            return (*this <=> other) != std::strong_ordering::equivalent;
        }
        bool operator>(const TagFilePath &other) const noexcept {
            return (*this <=> other) == std::strong_ordering::greater;
        }
        bool operator<(const TagFilePath &other) const noexcept {
            return (*this <=> other) == std::strong_ordering::less;
        }
    };
    
    /**
     * Attempt to open the file and read it all into a buffer
     * @param path path to the file
     * @return     a buffer holding the file or std::nullopt if failed
     */
    std::optional<std::vector<std::byte>> open_file(const std::filesystem::path &path);

    /**
     * Attempt to save the file
     * @param  path path to the file
     * @param  data data to write
     * @return      true on success; false on failure
     */
    bool save_file(const std::filesystem::path &path, const std::vector<std::byte> &data);

    /**
     * Convert a tag path to a file path for one tags directory. The file must exist, or std::nullopt will be returned.
     * @param  tag_path   tag path to use
     * @param  tags       tags directories to use
     * @return            file path or std::nullopt on failure
     */
    std::optional<std::filesystem::path> tag_path_to_file_path(const TagFilePath &tag_path, const std::vector<std::filesystem::path> &tags);

    /**
     * Convert a tag path to a file path for one tags directory. The file must exist, or std::nullopt will be returned.
     * @param  tag_path   tag path to use
     * @param  tags       tags directories to use
     * @return            file path or std::nullopt on failure
     */
    std::optional<std::filesystem::path> tag_path_to_file_path(const std::string &tag_path, const std::vector<std::filesystem::path> &tags);

    /**
     * Convert a tag path to a file path for one tags directory. The file does not have to exist.
     * @param  tag_path   tag path to use
     * @param  tags       tags directory to use
     * @return            file path or std::nullopt on failure
     */
    std::filesystem::path tag_path_to_file_path(const TagFilePath &tag_path, const std::filesystem::path &tags);

    /**
     * Convert a tag path to a file path for one tags directory. The file does not have to exist.
     * @param  tag_path   tag path to use
     * @param  tags       tags directory to use
     * @return            file path or std::nullopt on failure
     */
    std::filesystem::path tag_path_to_file_path(const std::string &tag_path, const std::filesystem::path &tags);

    /**
     * Convert a file path to a tag path relative to any of the tags directories. The file does not have to exist, but it must be relative to at least one tags directory.
     * @param  tag_path   tag file path to use
     * @param  tags       tags directories to use
     * @return            tag path or std::nullopt on failure
     */
    std::optional<std::string> file_path_to_tag_path(const std::filesystem::path &file_path, const std::vector<std::filesystem::path> &tags);

    /**
     * Convert a file path to a tag path relative to one tags directory. The file does not have to exist, but it must be relative to the tags directory.
     * @param  tag_path   tag file path to use
     * @param  tags       tags directory to use
     * @return            tag path or std::nullopt on failure
     */
    std::optional<std::string> file_path_to_tag_path(const std::filesystem::path &file_path, const std::filesystem::path &tags);

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
        HEK::TagFourCC tag_fourcc = {};

        /**
         * Split the tag path
         */
        std::vector<std::string> split_tag_path();
    };

    /**
     * Read a tags directory
     * @param  tags              tag directories
     * @param  filter_duplicates filter out duplicates (by default)
     * @param  status            optional pointer to a size_t to store the current number of tags loaded (for status messages)
     * @param  errors            optional pointer to hold the number of errors
     * @return                   all tags in the folder
     */
    std::vector<TagFile> load_virtual_tag_folder(const std::vector<std::filesystem::path> &tags, bool filter_duplicates = true, std::pair<std::mutex, std::size_t> *status = nullptr, std::size_t *errors = nullptr);

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
    
    /**
     * Check if the path matches (? matches any character, * matches any number of characters, / and \ match any path separator
     * @param path    path to check
     * @param pattern pattern to check
     * @return        true if a match was found
     */
    bool path_matches(const char *path, const char *pattern) noexcept;
    
    /**
     * Check if the path matches (? matches any character, * matches any number of characters, / and \ match any path separator
     * @param path    path to check
     * @param include include patterns to check (empty matches all)
     * @param exclude exclude patterns to check
     * @return        true if a match was found
     */
    bool path_matches(const char *path, const std::vector<std::string> &include, const std::vector<std::string> &exclude) noexcept;
}

#endif
