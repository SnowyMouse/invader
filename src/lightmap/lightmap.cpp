#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <vector>
#include <optional>

#include <invader/command_line_option.hpp>
#include <invader/printf.hpp>
#include <invader/file/file.hpp>
#include <invader/version.hpp>

#include "actions.hpp"

enum LightmapMode {
    LIGHTMAP_EXPORT,
    LIGHTMAP_IMPORT
};

int main(int argc, const char **argv) {
    Invader::setup_output();
    
    using namespace Invader;
    using namespace Invader::Lightmap;
    
    struct LightmapOptions {
        std::vector<std::filesystem::path> tags;
        //std::filesystem::path data = "data";
        bool filesystem_path = false;
        std::filesystem::path data = "data";
        
        std::optional<LightmapMode> mode;
    } shadowmouse_options;
    
    std::vector<CommandLineOption> options = {
        CommandLineOption("tags", 't', 1, "Use the specified tags directory. Additional tags directories can be specified for searching shaders, but the tag will be output to the first one.", "<dir>"),
        CommandLineOption("data", 'd', 1, "Use the specified data directory.", "<dir>"),
        CommandLineOption("export-mesh", 'E', 0, "Export a lightmap mesh to be imported and baked using an external program."),
        CommandLineOption("import-mesh", 'I', 0, "Import a lightmap mesh that was baked."),
        CommandLineOption("fs-path", 'P', 0, "Use a filesystem path for the tag."),
        CommandLineOption("info", 'i', 0, "Show credits, source info, and other info.")
    };

    static constexpr char DESCRIPTION[] = "Generate meshes to bake lightmaps using Blender's Cycles renderer.";
    static constexpr char USAGE[] = "<options> <scenario-path> <bsp-name>";

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<LightmapOptions &>(argc, argv, options, USAGE, DESCRIPTION, 2, 2, shadowmouse_options, [](char opt, const auto &args, LightmapOptions &shadowmouse_options) {
        switch(opt) {
            case 'd':
                shadowmouse_options.data = args[0];
                break;
            case 't':
                shadowmouse_options.tags.emplace_back(args[0]);
                break;
            case 'I':
                shadowmouse_options.mode = LightmapMode::LIGHTMAP_IMPORT;
                break;
            case 'E':
                shadowmouse_options.mode = LightmapMode::LIGHTMAP_EXPORT;
                break;
            case 'P':
                shadowmouse_options.filesystem_path = true;
                break;
            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);
                break;
        }
    });
    
    if(shadowmouse_options.tags.empty()) {
        shadowmouse_options.tags.emplace_back("tags");
    }
    
    if(!shadowmouse_options.mode.has_value()) {
        eprintf_error("No mode given. Use -h for more information");
        return EXIT_FAILURE;
    }
    
    // Resolve the bitmap tag
    std::string scenario_tag;
    if(shadowmouse_options.filesystem_path) {
        auto scenario_tag_maybe = File::file_path_to_tag_path(remaining_arguments[0], shadowmouse_options.tags);
        if(scenario_tag_maybe.has_value() && std::filesystem::exists(remaining_arguments[0])) {
            scenario_tag = std::filesystem::path(*scenario_tag_maybe).replace_extension().string();
        }
        else {
            eprintf_error("Failed to find a valid scenario %s in the tags directory.", remaining_arguments[0]);
            return EXIT_FAILURE;
        }
    }
    
    else {
        scenario_tag = remaining_arguments[0];
    }
    
    std::string bsp_name = remaining_arguments[1];
    std::filesystem::path mesh_file = (shadowmouse_options.data / scenario_tag).parent_path() / "lightmaps" / (bsp_name + ".lightmap_mesh");
    std::error_code ec;
    std::filesystem::create_directories(mesh_file.parent_path(), ec);
    
    switch(*shadowmouse_options.mode) {
        case LightmapMode::LIGHTMAP_EXPORT: {
            auto output = export_lightmap_mesh(scenario_tag.c_str(), bsp_name.c_str(), shadowmouse_options.tags);
            auto *data = reinterpret_cast<const std::byte *>(output.c_str());
            auto *data_end = data + output.size();
            if(!File::save_file(mesh_file, std::vector<std::byte>(data, data_end))) {
                eprintf_error("Failed to save %s", mesh_file.string().c_str());
                return EXIT_FAILURE;
            }
            oprintf_success("Saved %s", mesh_file.string().c_str());
            break;
        }
        case LightmapMode::LIGHTMAP_IMPORT: {
            auto input_bytes = Invader::File::open_file(mesh_file).value();
            auto input = std::string(reinterpret_cast<const char *>(input_bytes.data()), input_bytes.size());
            import_lightmap_mesh(input, mesh_file, scenario_tag.c_str(), bsp_name.c_str(), shadowmouse_options.tags);
            break;
        }
    }
}
