// SPDX-License-Identifier: GPL-3.0-only

#include <filesystem>
#include "../command_line_option.hpp"
#include <invader/file/file.hpp>
#include <invader/version.hpp>
#include <invader/printf.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/tag/parser/parser_struct.hpp>
#include <invader/tag/parser/compile/model.hpp>
#include <invader/tag/parser/compile/shader.hpp>
#include <invader/tag/parser/compile/object.hpp>

using namespace Invader;

template<typename T> static std::unique_ptr<Parser::ParserStruct> convert_object_template(Parser::ParserStruct &input) {
    std::unique_ptr<Parser::ParserStruct> output;
    output = std::make_unique<T>();
    Parser::convert_object(input, *output);
    return output;
}

// Get the conversion function
static std::optional<std::unique_ptr<Parser::ParserStruct> (*)(Parser::ParserStruct &)> conversion_function(HEK::TagFourCC from, HEK::TagFourCC to) noexcept {
    // We should't convert things to themselves
    if(from == to) {
        return std::nullopt;
    }
    
    if(IS_OBJECT_TAG(from) && IS_OBJECT_TAG(to)) {
        switch(to) {
            case HEK::TagFourCC::TAG_FOURCC_BIPED: return convert_object_template<Parser::Biped>;
            case HEK::TagFourCC::TAG_FOURCC_VEHICLE: return convert_object_template<Parser::Vehicle>;
            case HEK::TagFourCC::TAG_FOURCC_WEAPON: return convert_object_template<Parser::Weapon>;
            case HEK::TagFourCC::TAG_FOURCC_EQUIPMENT: return convert_object_template<Parser::Equipment>;
            case HEK::TagFourCC::TAG_FOURCC_GARBAGE: return convert_object_template<Parser::Garbage>;
            case HEK::TagFourCC::TAG_FOURCC_PROJECTILE: return convert_object_template<Parser::Projectile>;
            case HEK::TagFourCC::TAG_FOURCC_SCENERY: return convert_object_template<Parser::Scenery>;
            case HEK::TagFourCC::TAG_FOURCC_DEVICE_MACHINE: return convert_object_template<Parser::DeviceMachine>;
            case HEK::TagFourCC::TAG_FOURCC_DEVICE_CONTROL: return convert_object_template<Parser::DeviceControl>;
            case HEK::TagFourCC::TAG_FOURCC_DEVICE_LIGHT_FIXTURE: return convert_object_template<Parser::DeviceLightFixture>;
            case HEK::TagFourCC::TAG_FOURCC_PLACEHOLDER: return convert_object_template<Parser::Placeholder>;
            case HEK::TagFourCC::TAG_FOURCC_SOUND_SCENERY: return convert_object_template<Parser::SoundScenery>;
            default: break;
        }
    }
    
    // Others?
    switch(from) {
        case HEK::TagFourCC::TAG_FOURCC_MODEL:
            if(to == HEK::TagFourCC::TAG_FOURCC_GBXMODEL) {
                return [](Parser::ParserStruct &input) -> std::unique_ptr<Parser::ParserStruct> {
                    return std::make_unique<Parser::GBXModel>(Parser::convert_model_to_gbxmodel(dynamic_cast<Parser::Model &>(input)));
                };
            }
            break;
        case HEK::TagFourCC::TAG_FOURCC_GBXMODEL:
            if(to == HEK::TagFourCC::TAG_FOURCC_MODEL) {
                return [](Parser::ParserStruct &input) -> std::unique_ptr<Parser::ParserStruct> {
                    return std::make_unique<Parser::Model>(Parser::convert_gbxmodel_to_model(dynamic_cast<Parser::GBXModel &>(input)));
                };
            }
            break;
        case HEK::TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_CHICAGO_EXTENDED:
            if(to == HEK::TagFourCC::TAG_FOURCC_SHADER_TRANSPARENT_CHICAGO) {
                return [](Parser::ParserStruct &input) -> std::unique_ptr<Parser::ParserStruct> {
                    return std::make_unique<Parser::ShaderTransparentChicago>(Parser::convert_shader_transparent_chicago_extended_to_shader_transparent_chicago(dynamic_cast<Parser::ShaderTransparentChicagoExtended &>(input)));
                };
            }
            break;
        default:
            break;
    }
    return std::nullopt;
}

int main(int argc, const char **argv) {
    set_up_color_term();
    
    struct ConvertOptions {
        std::optional<std::pair<HEK::TagFourCC, HEK::TagFourCC>> conversion;
        
        std::filesystem::path tags = "tags";
        std::optional<std::filesystem::path> output_tags;
        bool overwrite = false;
        
        std::vector<std::string> batch, batch_exclude;
        
        
    } convert_options;

    const CommandLineOption options[] = {
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_INFO),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_TAGS),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_BATCH),
        CommandLineOption::from_preset(CommandLineOption::PRESET_COMMAND_LINE_OPTION_BATCH_EXCLUDE),
        CommandLineOption("overwrite", 'O', 0, "Overwrite any output tags if they exist."),
        CommandLineOption("output-tags", 'o', 1, "Set the output tags directory.", "<dir>"),
        CommandLineOption("groups", 'g', 2, "Set the conversion method.", "<from> <to>"),
    };

    static constexpr char DESCRIPTION[] = "Convert from one tag type to another.\n"
                                          "\n"
                                          "Supported conversions:\n"
                                          "  gbxmodel                            --> model\n"
                                          "  model                               --> gbxmodel\n"
                                          "  object (any)                        --> object (any)\n"
                                          "  shader_transparent_chicago_extended --> shader_transparent_chicago\n"
                                          "";
    static constexpr char USAGE[] = "[options] <-g <from> <to>> <-b <expr>|tag>";

    auto remaining_arguments = CommandLineOption::parse_arguments<ConvertOptions &>(argc, argv, options, USAGE, DESCRIPTION, 0, 1, convert_options, [](char opt, const auto &args, ConvertOptions &compare_options) {
        switch(opt) {
            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);
                
            case 'b':
                compare_options.batch.emplace_back(args[0]);
                break;
                
            case 'e':
                compare_options.batch_exclude.emplace_back(args[0]);
                break;
                
            case 'o':
                compare_options.output_tags = args[0];
                break;
                
            case 'O':
                compare_options.overwrite = true;
                break;
                
            case 't':
                compare_options.tags = args[0];
                break;
                
            case 'g':
                compare_options.conversion = { HEK::tag_extension_to_fourcc(args[0]), HEK::tag_extension_to_fourcc(args[1]) };
                
                if(compare_options.conversion->first == HEK::TagFourCC::TAG_FOURCC_NULL || compare_options.conversion->second == HEK::TagFourCC::TAG_FOURCC_NULL) {
                    eprintf_error("Invalid tag group pair %s and %s", args[0], args[1]);
                }
                
                break;
        }
    });
    
    // Make sure we have a conversion method specified
    if(!convert_options.conversion.has_value()) {
        eprintf_error("No conversion method specified. Use -h for more information.");
        return EXIT_FAILURE;
    }
    
    // Make sure we have tags to convert
    auto batching = !(convert_options.batch.empty() && convert_options.batch_exclude.empty());
    std::optional<std::string> tag_to_convert;
    if(batching) {
        if(!remaining_arguments.empty()) {
            eprintf_error("Cannot specify a single tag and do batching");
            return EXIT_FAILURE;
        }
    }
    else {
        if(remaining_arguments.empty()) {
            eprintf_error("A tag path was expected. Use -h for more information.");
            return EXIT_FAILURE;
        }
        tag_to_convert = remaining_arguments[0];
    }
    
    if(!convert_options.output_tags.has_value()) {
        convert_options.output_tags = convert_options.tags;
    }
    
    // Get the conversion function
    auto conversion_fn = conversion_function(convert_options.conversion->first, convert_options.conversion->second);
    if(!conversion_fn.has_value()) {
        eprintf_error("Cannot convert %s to %s", HEK::tag_fourcc_to_extension(convert_options.conversion->first), HEK::tag_fourcc_to_extension(convert_options.conversion->second));
        return EXIT_FAILURE;
    }
    
    // Get paths
    std::vector<std::filesystem::path> tags_vector;
    tags_vector.emplace_back(convert_options.tags);
    std::vector<File::TagFilePath> paths;
    if(batching) {
        for(auto &i : File::load_virtual_tag_folder(tags_vector)) {
            if(i.tag_fourcc == convert_options.conversion->first && File::path_matches((i.tag_path + "." + HEK::tag_fourcc_to_extension(convert_options.conversion->first)).c_str(), convert_options.batch, convert_options.batch_exclude)) {
                paths.emplace_back(File::split_tag_class_extension(File::halo_path_to_preferred_path(i.tag_path)).value());
            }
        }
    }
    else {
        paths.emplace_back(tag_to_convert.value(), convert_options.conversion->first);
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
        auto path_to = *convert_options.output_tags / File::TagFilePath(i.path, convert_options.conversion->second).join();
        
        try {
            auto tag_file = File::open_file(path_from);
            if(!tag_file.has_value()) {
                eprintf_error("Failed to read %s", path_from.string().c_str());
            }
            
            auto input_struct = Parser::ParserStruct::parse_hek_tag_file(tag_file->data(), tag_file->size());
            
            if(!convert_options.overwrite && std::filesystem::exists(path_to)) {
                eprintf_warn("Skipping %s...", i.join().c_str());
                continue;
            }
            
            // Convert it
            auto final_data = (*conversion_fn)(*input_struct)->generate_hek_tag_data(convert_options.conversion->second);
            
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
