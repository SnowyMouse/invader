// SPDX-License-Identifier: GPL-3.0-only

#ifdef _WIN32
#include <windows.h>
#endif

#include <invader/file/file.hpp>
#include <invader/printf.hpp>

#include <cstdio>
#include <filesystem>
#include <cstring>
#include <climits>

namespace Invader::File {
    std::optional<std::vector<std::byte>> open_file(const std::filesystem::path &path) {
        // Attempt to open it
        std::FILE *file = std::fopen(path.string().c_str(), "rb");
        if(!file) {
            return std::nullopt;
        }

        // Get the size
        std::vector<std::byte> file_data;

        // Workaround for filesystem::file_size() being screwed up on Windows when using files >2 GiB even though the damn thing returns a uintmax_t
        #ifdef _WIN32
        fseek(file, 0, SEEK_END);
        auto sizel = _ftelli64(file);
        if(sizel < 0) {
            std::fclose(file);
            return std::nullopt;
        }
        auto size = static_cast<std::size_t>(sizel);
        fseek(file, 0, SEEK_SET);

        // Get the size normally
        #else
        auto size = std::filesystem::file_size(path);
        #endif

        // Get the size and make sure we can use it
        if(size > file_data.max_size()) {
            std::fclose(file);
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

    bool save_file(const std::filesystem::path &path, const std::vector<std::byte> &data) {
        // Open the file
        std::FILE *f = std::fopen(path.string().c_str(), "wb");
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
    
    std::optional<std::filesystem::path> tag_path_to_file_path(const std::string &tag_path, const std::vector<std::filesystem::path> &tags) {
        for(auto &i : tags) {
            auto path = tag_path_to_file_path(tag_path, i);
            if(std::filesystem::exists(path)) {
                return path;
            }
        }
        return std::nullopt;
    }

    std::filesystem::path tag_path_to_file_path(const std::string &tag_path, const std::filesystem::path &tags) {
        return tags / halo_path_to_preferred_path(tag_path);
    }

    std::optional<std::string> file_path_to_tag_path(const std::filesystem::path &file_path, const std::filesystem::path &tags) {
        std::error_code ec;
        auto relative_path = std::filesystem::relative(file_path, tags, ec);
        
        // Nope?
        if(ec) {
            return std::nullopt;
        }
        
        // Get the top level directory
        auto relative_test = relative_path;
        while(relative_test.has_parent_path()) {
            relative_test = relative_test.parent_path();
        }
        
        // If we get this, then it isn't inside the actual directory
        if(relative_test == "..") {
            return std::nullopt;
        }
        
        // Done
        return relative_path.string();
    }
    
    std::optional<std::string> file_path_to_tag_path(const std::filesystem::path &file_path, const std::vector<std::filesystem::path> &tags) {
        for(auto &i : tags) {
            auto v = file_path_to_tag_path(file_path, i);
            if(v.has_value()) {
                return v;
            }
        }
        return std::nullopt;
    }

    constexpr char SYSTEM_PATH_SEPARATOR = INVADER_PREFERRED_PATH_SEPARATOR;
    constexpr char HALO_PATH_SEPARATOR = '\\';
    constexpr char PORTABLE_PATH_SEPARATOR = '/';

    void halo_path_to_preferred_path_chars(char *tag_path) noexcept {
        for(char *c = tag_path; *c != 0; c++) {
            if(*c == HALO_PATH_SEPARATOR || *c == SYSTEM_PATH_SEPARATOR || *c == PORTABLE_PATH_SEPARATOR) {
                *c = SYSTEM_PATH_SEPARATOR;
            }
        }
    }

    void preferred_path_to_halo_path_chars(char *tag_path) noexcept {
        for(char *c = tag_path; *c != 0; c++) {
            if(*c == HALO_PATH_SEPARATOR || *c == SYSTEM_PATH_SEPARATOR || *c == PORTABLE_PATH_SEPARATOR) {
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
            if(*tag_path == '\\' || *tag_path == '/' || *tag_path == INVADER_PREFERRED_PATH_SEPARATOR) {
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
            if(path[i] == '/' || path[i] == INVADER_PREFERRED_PATH_SEPARATOR || path[i] == '\\') {
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

    std::vector<TagFile> load_virtual_tag_folder(const std::vector<std::filesystem::path> &tags, std::pair<std::mutex, std::size_t> *status, std::size_t *errors) {
        std::vector<TagFile> all_tags;
        
        std::size_t new_errors = 0;

        std::pair<std::mutex, std::size_t> status_r;
        if(status == nullptr) {
            status = &status_r;
        }
        status->first.lock();
        status->second = 0;
        status->first.unlock();
        
        #define maybe_iterate_directories(file_path, ...) try { \
            iterate_directories(__VA_ARGS__); \
        } \
        catch(std::exception &e) { \
            eprintf_error("Error listing %s: %s", file_path.string().c_str(), e.what()); \
            new_errors++; \
        }
        
        // win32 implementation because Windows I/O is AWFUL
        #ifdef _WIN32
        auto iterate_directories = [&all_tags, &status, &new_errors](const std::filesystem::path &dir, auto &iterate_directories, int depth, std::size_t priority, const std::vector<std::filesystem::path> &main_dir) -> void {
            if(++depth == 256) {
                return;
            }
            
            WIN32_FIND_DATA find_data;
            HANDLE file = FindFirstFileA((dir / "*").string().c_str(), &find_data);
            bool found = file != nullptr;
            
            std::size_t tags_found = 0;
            
            while(found) {
                if(std::strcmp(find_data.cFileName, ".") != 0 && std::strcmp(find_data.cFileName, "..") != 0) {
                    auto file_path = dir / find_data.cFileName;
                    if(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        maybe_iterate_directories(file_path, file_path, iterate_directories, depth, priority, main_dir);
                    }
                    else {
                        auto extension = file_path.extension().string();
                        auto tag_fourcc = HEK::extension_to_tag_class(extension.c_str() + 1);

                        // First, make sure it's valid
                        if(tag_fourcc == HEK::TagClassInt::TAG_CLASS_NULL || tag_fourcc == HEK::TagClassInt::TAG_CLASS_NONE) {
                            goto spaghetti_next_tag;
                        }

                        // Next, add it
                        TagFile file;
                        file.full_path = file_path;
                        file.tag_fourcc = tag_fourcc;
                        file.tag_directory = priority;
                        file.tag_path = Invader::File::file_path_to_tag_path(file_path.string(), main_dir).value();
                        all_tags.emplace_back(std::move(file));
                        
                        tags_found++;
                    }
                }
                
                spaghetti_next_tag:
                found = FindNextFileA(file, &find_data);
            }
            
            // Update the find count
            if(tags_found) {
                status->first.lock();
                status->second += tags_found;
                status->first.unlock();
            }
        };
        #else
        auto iterate_directories = [&all_tags, &status, &new_errors](const std::filesystem::path &dir, auto &iterate_directories, int depth, std::size_t priority, const std::vector<std::filesystem::path> &main_dir) -> void {
            if(++depth == 256) {
                return;
            }
            
            std::size_t tags_found = 0;

            for(auto &d : std::filesystem::directory_iterator(dir)) {
                auto file_path = d.path();
                
                if(d.is_directory()) {
                    maybe_iterate_directories(file_path, d, iterate_directories, depth, priority, main_dir);
                }
                else if(file_path.has_extension() && std::filesystem::is_regular_file(file_path)) {
                    auto extension = file_path.extension().string();
                    auto tag_fourcc = HEK::extension_to_tag_class(extension.c_str() + 1);

                    // First, make sure it's valid
                    if(tag_fourcc == HEK::TagClassInt::TAG_CLASS_NULL || tag_fourcc == HEK::TagClassInt::TAG_CLASS_NONE) {
                        continue;
                    }

                    // Next, add it
                    TagFile file;
                    file.full_path = file_path;
                    file.tag_fourcc = tag_fourcc;
                    file.tag_directory = priority;
                    file.tag_path = Invader::File::file_path_to_tag_path(file_path.string(), main_dir).value();
                    all_tags.emplace_back(std::move(file));
                    tags_found++;
                }
            }
            
            // Update the find count
            if(tags_found) {
                status->first.lock();
                status->second += tags_found;
                status->first.unlock();
            }
        };
        #endif

        // Go through each directory
        std::size_t dir_count = tags.size();
        for(std::size_t i = 0; i < dir_count; i++) {
            auto &d = tags[i];
            
            maybe_iterate_directories(d, d, iterate_directories, 0, i, std::vector<std::filesystem::path>(&tags[i], &tags[i] + 1));
        }
        
        // Change error count if errors was specified
        if(errors) {
            *errors = new_errors;
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
            if((this_i == '\\' || this_i == '/' || this_i == INVADER_PREFERRED_PATH_SEPARATOR) && (next_i == '\\' || next_i == '/' || next_i == INVADER_PREFERRED_PATH_SEPARATOR)) {
                for(char *j = i + 1; *j; j++) {
                    j[0] = j[1];
                }
            }
        }
    }
    
    void check_working_directory(const char *file) {
        // lol
        time_t t = time(nullptr);
        struct tm tm = *localtime(&t);
        if(tm.tm_mday == 1 && tm.tm_mon == 3) {
            if(std::filesystem::exists(file)) {
                oprintf_success("Successfully loaded map file '%s'", file);
                eprintf_warn("You still need to set your working directory.");
            }
            else {
                eprintf_warn("WARNING: Couldn't read map file '%s'", file);
                eprintf_warn("You need to set your working directory.");
            }
        }
    }
    
    bool path_matches(const char *path, const char *pattern) {
        for(const char *p = pattern;; p++) {
            // End of string
            if(*p == *path && *p == 0) {
                return true;
            }
            else if(
                (*p == '?' || *p == *path) || 
                ((*p == '/' || *p == '\\' || *p == INVADER_PREFERRED_PATH_SEPARATOR) && (*path == '\\' || *path == '/' || *path == INVADER_PREFERRED_PATH_SEPARATOR))
            ) {
                path++;
                continue;
            }
            else if(*p == '*') {
                // Skip duplicates
                while(*p == '*') {
                    p++;
                }
                if(*p == 0) {
                    return true;
                }
                for(const char *s = path; *s; s++) {
                    if(path_matches(s, p)) {
                        return true;
                    }
                }
                return false;
            }
            else {
                return false;
            }
        }
        return true;
    }
}
