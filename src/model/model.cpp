// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <cstring>
#include <regex>
#include <map>

#include <invader/version.hpp>
#include <invader/printf.hpp>
#include <invader/file/file.hpp>
#include <invader/command_line_option.hpp>
#include <invader/model/jms.hpp>

enum ModelType {
    MODEL_TYPE_MODEL = 0,
    MODEL_TYPE_GBXMODEL
};

const char *MODEL_EXTENSIONS[] = {
    ".model",
    ".gbxmodel"
};

int main(int argc, const char **argv) {
    using namespace Invader;
    using namespace Invader::HEK;
    
    struct ModelOptions {
        std::optional<ModelType> type;
        std::vector<std::filesystem::path> tags;
        std::filesystem::path data = "data";
        bool filesystem_path = false;
        bool legacy = false;
    } model_options;

    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("legacy", 'L', 0, "Use legacy behavior (models directory; cannot use -P with a data directory).");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the tag path or data directory.");
    options.emplace_back("type", 'T', 1, "Specify the type of model. Can be: model, gbxmodel", "<type>");
    options.emplace_back("data", 'd', 1, "Use the specified data directory.", "<dir>");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory. Additional tags directories can be specified for searching shaders, but the tag will be output to the first one.", "<dir>");

    static constexpr char DESCRIPTION[] = "Create a model tag.";
    static constexpr char USAGE[] = "[options] <model-tag>";

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<ModelOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, model_options, [](char opt, const auto &args, ModelOptions &model_options) {
        switch(opt) {
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
            case 'P':
                model_options.filesystem_path = true;
                break;
            case 'L':
                model_options.legacy = true;
                break;
            case 'T':
                if(std::strcmp(args[0], "model") == 0) {
                    model_options.type = ModelType::MODEL_TYPE_MODEL;
                }
                else if(std::strcmp(args[0], "gbxmodel") == 0) {
                    model_options.type = ModelType::MODEL_TYPE_GBXMODEL;
                }
                else {
                    eprintf_error("Invalid type %s", args[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
        }
    });
    
    if(model_options.tags.size() == 0) {
        model_options.tags.emplace_back("tags");
    }
    
    // Do this
    if(!model_options.type.has_value()) {
        eprintf_error("No type specified. Use -h for more information.");
        return EXIT_FAILURE;
    }
    
    const char *extension = MODEL_EXTENSIONS[*model_options.type];
    
    // Handle -P
    std::string model_tag;
    if(model_options.filesystem_path) {
        auto model_tag_maybe = File::file_path_to_tag_path(remaining_arguments[0], model_options.tags);
        if(model_tag_maybe.has_value() && std::filesystem::exists(remaining_arguments[0])) {
            auto path = std::filesystem::path(*model_tag_maybe);
            if(model_options.legacy) {
                path = path / (path.parent_path().filename().string() + extension);
            }
            if(path.extension() == extension) {
                model_tag = path.replace_extension().string();
            }
            else {
                eprintf_error("Extension must be %s", remaining_arguments[0]);
            }
        }
        else if(model_options.legacy) {
            auto model_folder_maybe = File::file_path_to_tag_path(remaining_arguments[0], model_options.data);
            if(model_folder_maybe.has_value() && std::filesystem::exists(remaining_arguments[0])) {
                model_tag = *model_folder_maybe;
            }
            else {
                eprintf_error("Failed to find a valid model %s in the data or tags directories.", remaining_arguments[0]);
        return EXIT_FAILURE;
            }
        }
        else {
            eprintf_error("Failed to find a valid model %s in the tags directories.", remaining_arguments[0]);
        return EXIT_FAILURE;
        }
    }
    else {
        model_tag = remaining_arguments[0];
        if(model_options.legacy) {
            auto tp = std::filesystem::path(model_tag);
            model_tag = (tp / tp.parent_path().filename()).string();
        }
    }
    
    // Let's do this
    std::map<std::string, JMS> jms_files;
    std::filesystem::path directory = model_options.data / model_tag;
    if(model_options.legacy) {
        directory = directory.parent_path() / "models";
    }
    
    // Does it exist?
    if(!std::filesystem::is_directory(directory)) {
        eprintf_error("No directory exists at %s", directory.string().c_str());
        return EXIT_FAILURE;
    }
    
    // Let's do this
    try {
        // Get paths. Sort alphabetically.
        for(auto &i : std::filesystem::directory_iterator(directory)) {
            auto path = i.path();
            if(path.extension() == ".jms" && i.is_regular_file()) {
                try {
                    auto file = File::open_file(path);
                    if(!file.has_value()) {
                        eprintf_error("Failed to read %s", path.string().c_str());
                        return EXIT_FAILURE;
                    }
                    jms_files.emplace(path.filename().replace_extension(), JMS::from_string(std::string(reinterpret_cast<const char *>(file->data()), file->size()).c_str()));
                }
                catch(std::exception &) {
                    eprintf_error("Failed to parse %s", path.string().c_str());
                    return EXIT_FAILURE;
                }
            }
        }
    }
    catch(std::exception &e) {
        eprintf_error("Failed to iterate through %s: %s", directory.string().c_str(), e.what());
        return EXIT_FAILURE;
    }
    
    // Nothing found?
    if(jms_files.empty()) {
        eprintf_error("No .jms files found in %s", directory.c_str());
        return EXIT_FAILURE;
    }
    
}
