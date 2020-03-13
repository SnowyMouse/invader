// SPDX-License-Identifier: GPL-3.0-only

#include <invader/file/file.hpp>

#include <cstdio>
#include <filesystem>
#include <cstring>
#include <climits>

namespace Invader::File {
    std::optional<std::vector<std::byte>> open_file(const char *path) {
        // Attempt to open it
        std::FILE *file = std::fopen(path, "rb");
        if(!file) {
            return std::nullopt;
        }

        // Get the size
        std::vector<std::byte> file_data;
        auto size = std::filesystem::file_size(std::filesystem::path(path));

        // Get the size and make sure we can use it
        if(size > file_data.max_size()) {
            return std::nullopt;
        }

        // Read it
        file_data.resize(static_cast<std::size_t>(size));

        auto *data = file_data.data();
        std::size_t offset = 0;

        while(offset < size) {
            long amount_to_read = (size - offset) > LONG_MAX ? LONG_MAX : (size - offset);
            if(std::fread(data, amount_to_read, 1, file) != 1) {
                std::fclose(file);
                return std::nullopt;
            }

            data += amount_to_read;
            offset += amount_to_read;
        }

        // Return what we got
        std::fclose(file);
        return file_data;
    }

    bool save_file(const char *path, const std::vector<std::byte> &data) {
        // Open the file
        std::FILE *f = std::fopen(path, "wb");
        if(!f) {
            return false;
        }

        // Write
        if(std::fwrite(data.data(), data.size(), 1, f) != 1) {
            std::fclose(f);
            return false;
        }

        std::fclose(f);
        return true;
    }

    std::optional<std::string> tag_path_to_file_path(const std::string &tag_path, const std::vector<std::string> &tags, bool must_exist) {
        // if it's an absolute path, we can't do anything about this
        std::filesystem::path tag_path_path(tag_path);
        if(tag_path_path.is_absolute()) {
            return std::nullopt;
        }

        for(auto &tags_directory_string : tags) {
            std::filesystem::path tags_directory(tags_directory_string);
            auto file_path = tags_directory / tag_path_path;
            if(!must_exist || std::filesystem::exists(file_path)) {
                return file_path.string();
            }
        }
        return std::nullopt;
    }

    std::optional<std::string> file_path_to_tag_path(const std::string &file_path, const std::vector<std::string> &tags, bool must_exist) {
        // Get the absolute file path. It doesn't matter if it exists.
        auto absolute_file_path = std::filesystem::absolute(file_path);
        if(must_exist && !std::filesystem::exists(absolute_file_path)) {
            return std::nullopt;
        }
        auto absolute_file_path_str = absolute_file_path.string();
        auto root_path_str = absolute_file_path.root_path();

        for(auto &tags_directory_string : tags) {
            // Get the absolute path
            bool ends_with_separator = tags_directory_string[tags_directory_string.size() - 1] == '/' || tags_directory_string[tags_directory_string.size() - 1] == std::filesystem::path::preferred_separator;
            auto tags_directory = std::filesystem::absolute(ends_with_separator ? tags_directory_string.substr(0,tags_directory_string.size() - 1) : tags_directory_string);

            // Go back until we get something that's the same
            auto path_check = absolute_file_path;
            while(path_check.has_parent_path() && path_check != root_path_str) {
                path_check = path_check.parent_path();
                if(path_check == tags_directory) {
                    auto path_check_str = path_check.string();
                    auto path_check_size = path_check_str.size();
                    auto converted_path = absolute_file_path_str.substr(path_check_size + 1, absolute_file_path_str.size() - path_check_size - 1);
                    return converted_path;
                }
            }
        }
        return std::nullopt;
    }

    std::optional<std::string> file_path_to_tag_path_with_extension(const std::string &tag_path, const std::vector<std::string> &tags, const std::string &expected_extension) {
        std::size_t EXTENSION_LENGTH = expected_extension.size();
        std::size_t TAG_PATH_LENGTH = tag_path.size();

        // If the path is too small to have the extension, no dice
        if(EXTENSION_LENGTH >= tag_path.size()) {
            return std::nullopt;
        }

        // Otherwise, check if we have the extension
        if(TAG_PATH_LENGTH > EXTENSION_LENGTH && std::strcmp(tag_path.c_str() + TAG_PATH_LENGTH - EXTENSION_LENGTH, expected_extension.c_str()) == 0) {
            // If the user simply put ".scenario" at the end, remove it
            if(Invader::File::tag_path_to_file_path(tag_path, tags, true).has_value()) {
                return tag_path.substr(0, TAG_PATH_LENGTH - EXTENSION_LENGTH);
            }

            // Otherwise see if we can find it
            else {
                auto tag_maybe = Invader::File::file_path_to_tag_path(tag_path, tags, true);
                if(tag_maybe.has_value()) {
                    auto &tag = tag_maybe.value();
                    return tag.substr(0, tag.size() - EXTENSION_LENGTH);
                }
                else {
                    return std::nullopt;
                }
            }
        }

        // We don't? Give up.
        return std::nullopt;
    }

    constexpr char SYSTEM_PATH_SEPARATOR = std::filesystem::path::preferred_separator;
    constexpr char HALO_PATH_SEPARATOR = '\\';
    constexpr char PORTABLE_PATH_SEPARATOR = '/';

    void halo_path_to_preferred_path_chars(char *tag_path) noexcept {
        if(SYSTEM_PATH_SEPARATOR != HALO_PATH_SEPARATOR) {
            for(char *c = tag_path; *c != 0; c++) {
                if(*c == HALO_PATH_SEPARATOR) {
                    *c = SYSTEM_PATH_SEPARATOR;
                }
            }
        }
    }

    void preferred_path_to_halo_path_chars(char *tag_path) noexcept {
        for(char *c = tag_path; *c != 0; c++) {
            if(*c == SYSTEM_PATH_SEPARATOR || *c == PORTABLE_PATH_SEPARATOR) {
                *c = HALO_PATH_SEPARATOR;
            }
        }
    }

    std::string halo_path_to_preferred_path(const std::string &tag_path) {
        const char *tag_path_c = tag_path.c_str();
        std::vector<char> tag_path_cv(tag_path_c, tag_path_c + tag_path.size() + 1);
        halo_path_to_preferred_path_chars(tag_path_cv.data());
        return tag_path_cv.data();
    }

    std::string preferred_path_to_halo_path(const std::string &tag_path) {
        const char *tag_path_c = tag_path.c_str();
        std::vector<char> tag_path_cv(tag_path_c, tag_path_c + tag_path.size() + 1);
        preferred_path_to_halo_path_chars(tag_path_cv.data());
        return tag_path_cv.data();
    }

    const char *base_name_chars(const char *tag_path) noexcept {
        const char *base_name = tag_path;
        while(*tag_path) {
            if(*tag_path == '\\' || *tag_path == '/' || *tag_path == std::filesystem::path::preferred_separator) {
                base_name = tag_path + 1;
            }
            tag_path++;
        }
        return base_name;
    }

    char *base_name_chars(char *tag_path) noexcept {
        return const_cast<char *>(base_name_chars(const_cast<const char *>(tag_path)));
    }

    std::string base_name(const std::string &tag_path, bool drop_extension) {
        const char *base_name = base_name_chars(tag_path.c_str());

        if(drop_extension) {
            const char *base_name_end = nullptr;
            for(const char *c = base_name; *c; c++) {
                if(*c == '.') {
                    base_name_end = c;
                }
            }
            if(base_name_end) {
                return std::string(base_name, base_name_end);
            }
        }
        return std::string(base_name);
    }

    std::string remove_trailing_slashes(const std::string &path) {
        const char *path_str = path.c_str();
        std::vector<char> v = std::vector<char>(path_str, path_str + path.size() + 1);
        v.push_back(0);
        remove_trailing_slashes_chars(v.data());
        return std::string(v.data());
    }

    void remove_trailing_slashes_chars(char *path) {
        long path_length = static_cast<long>(std::strlen(path));
        for(long i = path_length - 1; i >= 0; i++) {
            if(path[i] == '/' || path[i] == std::filesystem::path::preferred_separator) {
                path[i] = 0;
            }
            else {
                break;
            }
        }
    }

    std::optional<TagFilePath> split_tag_class_extension(const std::string &tag_path) {
        return split_tag_class_extension_chars(tag_path.c_str());
    }

    std::optional<TagFilePath> split_tag_class_extension_chars(const char *tag_path) {
        const char *extension = nullptr;
        for(const char *c = tag_path; *c; c++) {
            if(*c == '.') {
                extension = c + 1;
            }
        }
        if(!extension) {
            return std::nullopt;
        }

        auto tag_class = HEK::extension_to_tag_class(extension);
        if(tag_class == TagClassInt::TAG_CLASS_NONE || tag_class == TagClassInt::TAG_CLASS_NULL) {
            return std::nullopt;
        }
        else {
            return TagFilePath { std::string(tag_path, (extension - 1) - tag_path), HEK::extension_to_tag_class(extension) };
        }
    }

    std::vector<TagFile> load_virtual_tag_folder(const std::vector<std::string> &tags, std::pair<std::mutex, std::size_t> *status) {
        std::vector<TagFile> all_tags;

        std::pair<std::mutex, std::size_t> status_r;
        if(status == nullptr) {
            status = &status_r;
        }
        status->first.lock();
        status->second = 0;
        status->first.unlock();

        auto iterate_directories = [&all_tags, &status](const std::vector<std::string> &the_story_thus_far, const std::filesystem::path &dir, auto &iterate_directories, int depth, std::size_t priority, const std::vector<std::string> &main_dir) -> void {
            if(++depth == 256) {
                return;
            }

            for(auto &d : std::filesystem::directory_iterator(dir)) {
                std::vector<std::string> add_dir = the_story_thus_far;
                auto file_path = d.path();
                add_dir.emplace_back(file_path.filename().string());
                if(d.is_directory()) {
                    iterate_directories(add_dir, d, iterate_directories, depth, priority, main_dir);
                }
                else if(file_path.has_extension()) {
                    auto extension = file_path.extension().string();
                    auto tag_class_int = HEK::extension_to_tag_class(extension.c_str() + 1);

                    // First, make sure it's valid
                    if(tag_class_int == HEK::TagClassInt::TAG_CLASS_NULL || tag_class_int == HEK::TagClassInt::TAG_CLASS_NONE) {
                        continue;
                    }

                    // Next, add it
                    TagFile file;
                    file.full_path = file_path;
                    file.tag_class_int = tag_class_int;
                    file.tag_directory = priority;
                    file.tag_path = Invader::File::file_path_to_tag_path(file_path.string(), main_dir, false).value();
                    all_tags.emplace_back(std::move(file));
                    status->first.lock();
                    status->second++;
                    status->first.unlock();
                }
            }
        };

        // Go through each directory
        std::size_t dir_count = tags.size();
        for(std::size_t i = 0; i < dir_count; i++) {
            auto &d = tags[i];
            auto dir_str = d.c_str();
            iterate_directories(std::vector<std::string>(), d, iterate_directories, 0, i, std::vector<std::string>(&dir_str, &dir_str + 1));
        }

        return all_tags;
    }

    std::vector<std::string> TagFile::split_tag_path() {
        std::vector<std::string> elements;
        auto halo_path = preferred_path_to_halo_path(this->tag_path);

        const char *word = halo_path.c_str();
        for(const char *c = halo_path.c_str() + 1; *c; c++) {
            if(*c == '\\') {
                // Make a new string with this element and add it
                elements.emplace_back(word, c - word);

                // Fast forward through any duplicate backslashes
                while(*c == '\\') {
                    c++;
                }

                // Go back one so the iterator can handle this
                c--;
            }
        }

        // Add the last element
        elements.emplace_back(word);

        return elements;
    }


   std::string remove_duplicate_slashes(const std::string &path) {
       const char *tag_path_c = path.c_str();
       std::vector<char> tag_path_cv(tag_path_c, tag_path_c + path.size() + 1);
       remove_duplicate_slashes_chars(tag_path_cv.data());
       return tag_path_cv.data();
   }

   void remove_duplicate_slashes_chars(char *path) {
       for(char *i = path; *i; i++) {
           char this_i = i[0];
           char next_i = i[1];
           if((this_i == '\\' || this_i == '/' || this_i == std::filesystem::path::preferred_separator) && (next_i == '\\' || next_i == '/' || next_i == std::filesystem::path::preferred_separator)) {
               for(char *j = i + 1; *j; j++) {
                   j[0] = j[1];
               }
           }
       }
   }
}
