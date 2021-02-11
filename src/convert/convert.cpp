// SPDX-License-Identifier: GPL-3.0-only

#include <filesystem>
#include <invader/command_line_option.hpp>
#include <invader/file/file.hpp>
#include <invader/version.hpp>
#include <invader/printf.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/tag/parser/parser_struct.hpp>
#include <invader/tag/parser/compile/model.hpp>
#include <invader/tag/parser/compile/shader.hpp>

using namespace Invader;

enum Conversion {
    GBXMODEL_TO_MODEL,
    MODEL_TO_GBXMODEL,
    CHICAGO_EXTENDED_TO_CHICAGO
};

int main(int argc, const char **argv) {
    struct ConvertOptions {
        std::optional<Conversion> conversion;
        std::filesystem::path tags = "tags";
        std::optional<std::filesystem::path> output_tags;
        bool match_all = false;
        bool overwrite = false;
        bool use_filesystem_path = false;
        std::vector<std::string> tags_to_convert;
    } convert_options;

    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("all", 'a', 0, "Convert all tags. This cannot be used with -s.");
    options.emplace_back("single-tag", 's', 1, "Convert a specific tag. This can be specified multiple times for multiple tags, but cannot be used with -a.");
    options.emplace_back("overwrite", 'O', 0, "Overwrite any output tags if they exist.");
    options.emplace_back("tags", 't', 1, "Set the input tags directory.", "<dir>");
    options.emplace_back("output-tags", 'o', 1, "Set the output tags directory.", "<dir>");
    options.emplace_back("type", 'T', 1, "Type of conversion. Can be: gbxmodel-to-model (g2m), model-to-gbxmodel (m2g), chicago-extended-to-chicago (x2c)", "<type>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the tag path if specifying a tag.");

    static constexpr char DESCRIPTION[] = "Convert from one tag type to another.";
    static constexpr char USAGE[] = "[options] <--all | -s <tag>>";

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<ConvertOptions &>(argc, argv, options, USAGE, DESCRIPTION, 0, 0, convert_options, [](char opt, const auto &args, ConvertOptions &compare_options) {
        switch(opt) {
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
                
            case 'a':
                compare_options.match_all = true;
                break;
                
            case 'T':
                if(std::strcmp(args[0], "gbxmodel-to-model") == 0 || std::strcmp(args[0], "g2m") == 0) {
                    compare_options.conversion = Conversion::GBXMODEL_TO_MODEL;
                }
                else if(std::strcmp(args[0], "model-to-gbxmodel") == 0 || std::strcmp(args[0], "m2g") == 0) {
                    compare_options.conversion = Conversion::MODEL_TO_GBXMODEL;
                }
                else if(std::strcmp(args[0], "chicago-extended-to-chicago") == 0 || std::strcmp(args[0], "x2c") == 0) {
                    compare_options.conversion = Conversion::CHICAGO_EXTENDED_TO_CHICAGO;
                }
                else {
                    eprintf_error("Unknown conversion type %s", args[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
                
            case 'o':
                compare_options.output_tags = args[0];
                break;
                
            case 'P':
                compare_options.use_filesystem_path = true;
                break;
                
            case 'O':
                compare_options.overwrite = true;
                break;
                
            case 's':
                compare_options.tags_to_convert.emplace_back(args[0]);
                break;
                
            case 't':
                compare_options.tags = args[0];
                break;
        }
    });
    
    // Make sure we have a conversion method specified
    if(!convert_options.conversion.has_value()) {
        eprintf_error("No conversion type specified. Use -h for more information.");
        return EXIT_FAILURE;
    }
    
    // Make sure we have tags to convert
    if(convert_options.tags_to_convert.size() == 0 && convert_options.match_all == false) {
        eprintf_error("No input tags provided. Use --all for all tags.");
        return EXIT_FAILURE;
    }
    
    if(convert_options.tags_to_convert.size() > 0 && convert_options.match_all) {
        eprintf_error("-a cannot be used with -s");
        return EXIT_FAILURE;
    }
    
    if(!convert_options.output_tags.has_value()) {
        convert_options.output_tags = convert_options.tags;
    }
    
    HEK::TagClassInt input_class, output_class;
    
    switch(*convert_options.conversion) {
        case Conversion::GBXMODEL_TO_MODEL:
            input_class = HEK::TagClassInt::TAG_CLASS_GBXMODEL;
            output_class = HEK::TagClassInt::TAG_CLASS_MODEL;
            break;
        case Conversion::MODEL_TO_GBXMODEL:
            input_class = HEK::TagClassInt::TAG_CLASS_MODEL;
            output_class = HEK::TagClassInt::TAG_CLASS_GBXMODEL;
            break;
        case Conversion::CHICAGO_EXTENDED_TO_CHICAGO:
            input_class = HEK::TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_CHICAGO_EXTENDED;
            output_class = HEK::TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_CHICAGO;
            break;
        default:
            std::terminate();
    }
    
    // Get paths
    std::vector<std::filesystem::path> tags_vector;
    tags_vector.emplace_back(convert_options.tags);
    std::vector<File::TagFilePath> paths;
    if(convert_options.match_all) {
        for(auto &i : File::load_virtual_tag_folder(tags_vector)) {
            if(i.tag_class_int == input_class) {
                paths.emplace_back(File::split_tag_class_extension(File::halo_path_to_preferred_path(i.tag_path)).value());
            }
        }
    }
    else if(convert_options.use_filesystem_path) {
        for(auto &i : convert_options.tags_to_convert) {
            try {
                paths.emplace_back(File::split_tag_class_extension(File::halo_path_to_preferred_path(File::file_path_to_tag_path(i, tags_vector).value())).value());
            }
            catch(std::exception &) {
                eprintf_error("Invalid tag path %s", i.c_str());
                return EXIT_FAILURE;
            }
        }
    }
    else {
        for(auto &i : convert_options.tags_to_convert) {
            try {
                paths.emplace_back(File::split_tag_class_extension(File::halo_path_to_preferred_path(i)).value());
            }
            catch(std::exception &) {
                eprintf_error("Invalid tag path %s", i.c_str());
                return EXIT_FAILURE;
            }
        }
    }
    
    // Make sure they all exist
    for(auto &i : paths) {
        auto path = convert_options.tags / i.join();
        if(!std::filesystem::exists(path)) {
            eprintf_error("Tag file %s does not exist", path.string().c_str());
            return EXIT_FAILURE;
        }
    }
    
    // Let's begin
    std::size_t success = 0;
    std::size_t total = paths.size();
    for(auto &i : paths) {
        auto path_from = convert_options.tags / i.join();
        auto path_to = *convert_options.output_tags / File::TagFilePath(i.path, output_class).join();
        
        try {
            auto tag_file = File::open_file(path_from);
            if(!tag_file.has_value()) {
                eprintf_error("Failed to read %s", path_from.string().c_str());
            }
            
            auto input_struct = Parser::ParserStruct::parse_hek_tag_file(tag_file->data(), tag_file->size());
            std::unique_ptr<Parser::ParserStruct> output_struct;
            
            if(!convert_options.overwrite && std::filesystem::exists(path_to)) {
                eprintf_warn("Skipping %s...", i.join().c_str());
                continue;
            }
            
            switch(*convert_options.conversion) {
                case GBXMODEL_TO_MODEL:
                    output_struct = std::make_unique<Parser::Model>(Parser::convert_gbxmodel_to_model(dynamic_cast<Parser::GBXModel &>(*input_struct)));
                    break;
                case MODEL_TO_GBXMODEL:
                    output_struct = std::make_unique<Parser::GBXModel>(Parser::convert_model_to_gbxmodel(dynamic_cast<Parser::Model &>(*input_struct)));
                    break;
                case CHICAGO_EXTENDED_TO_CHICAGO:
                    output_struct = std::make_unique<Parser::ShaderTransparentChicago>(Parser::convert_shader_transparent_chicago_extended_to_shader_transparent_chicago(dynamic_cast<Parser::ShaderTransparentChicagoExtended &>(*input_struct)));
                    break;
            }
            
            // Build
            auto final_data = output_struct->generate_hek_tag_data(output_class);
            
            // Make directories
            std::error_code ec;
            std::filesystem::create_directories(path_to.parent_path(), ec);

            // Save
            if(File::save_file(path_to, final_data)) {
                oprintf_success("Saved %s", path_to.string().c_str());
                success++;
            }
            else {
                eprintf_error("Failed to write to %s", path_to.string().c_str());
            }
        }
        catch(std::exception &e) {
            eprintf_error("Failed to convert %s: %s", i.join().c_str(), e.what());
        }
    }
    
    // Report results
    if(success) {
        oprintf_success("Converted %zu of %zu tag%s", success, total, total == 1 ? "" : "s");
    }
    else {
        oprintf("Converted %zu of %zu tag%s\n", success, total, total == 1 ? "" : "s");
    }
}
