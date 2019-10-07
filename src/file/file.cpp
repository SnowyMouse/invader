// SPDX-License-Identifier: GPL-3.0-only

#include "file.hpp"

#include <cstdio>
#include <filesystem>

namespace Invader::File {
    std::optional<std::vector<std::byte>> open_file(const char *path) {
        // Get the file
        std::filesystem::path file_path(path);

        // Make sure we're dealing with a file we can open
        if(std::filesystem::is_regular_file(file_path)) {
            return std::nullopt;
        }

        // Get the size
        auto sizeb = std::filesystem::file_size(file_path);

        // Get the size and make sure we can use it
        if(sizeb > SIZE_MAX) {
            return std::nullopt;
        }
        auto size = static_cast<std::size_t>(sizeb);

        // Attempt to open it
        std::FILE *file = std::fopen(path, "rb");
        if(!file) {
            return std::nullopt;
        }

        // Read it
        std::vector<std::byte> file_data(static_cast<std::size_t>(size));
        if(std::fread(file_data.data(), size, 1, file) != 1) {
            std::fclose(file);
            return std::nullopt;
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
};
