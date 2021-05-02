#ifndef INVADER__LIGHTMAP__LIGHTMAP_HPP
#define INVADER__LIGHTMAP__LIGHTMAP_HPP

#include <vector>
#include <string>
#include <filesystem>

namespace Invader::Lightmap {
    std::string export_lightmap_mesh(const char *scenario, const char *bsp_name, const std::vector<std::filesystem::path> &tags_directories);
    void import_lightmap_mesh(const std::string &mesh_data, const std::filesystem::path &mesh_path, const char *scenario, const char *bsp_name, const std::vector<std::filesystem::path> &tags_directories);
}

#endif
