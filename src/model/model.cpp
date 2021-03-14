// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <cstring>
#include <regex>
#include <list>

#include <invader/version.hpp>
#include <invader/printf.hpp>
#include <invader/file/file.hpp>
#include <invader/command_line_option.hpp>
#include <invader/model/jms.hpp>
#include <invader/tag/parser/parser.hpp>

enum ModelType {
    MODEL_TYPE_MODEL = 0,
    MODEL_TYPE_GBXMODEL
};

const char *MODEL_EXTENSIONS[] = {
    ".model",
    ".gbxmodel"
};

template <typename T, Invader::HEK::TagFourCC fourcc> std::vector<std::byte> make_model_tag(const std::filesystem::path &path, const std::vector<std::filesystem::path> &tags, const Invader::JMSMap &map) {
    using namespace Invader;
    
    // Load the tag if possible
    std::unique_ptr<Parser::ParserStruct> tag;
    if(std::filesystem::exists(path)) {
        auto old_file = File::open_file(path);
        if(old_file.has_value()) {
            try {
                tag = Parser::ParserStruct::parse_hek_tag_file(old_file->data(), old_file->size());
            }
            catch(std::exception &e) {
                eprintf_error("Failed to parse %s: %s", path.string().c_str(), e.what());
                std::exit(EXIT_FAILURE);
            }
        }
        else {
            eprintf_error("Failed to open %s", path.string().c_str());
            std::exit(EXIT_FAILURE);
        }
    }
    else {
        tag = Parser::ParserStruct::generate_base_struct(fourcc);
    }
    
    // Is it valid?
    auto *model_tag = dynamic_cast<T *>(tag.get());
    if(model_tag == nullptr) {
        eprintf_error("Failed to parse %s (probably not a %s tag)", path.string().c_str(), HEK::tag_fourcc_to_extension(fourcc));
        std::exit(EXIT_FAILURE);
    }
    
    // Clear this stuff
    model_tag->markers.clear();
    model_tag->nodes.clear();
    model_tag->regions.clear();
    model_tag->geometries.clear();
    model_tag->shaders.clear();
    model_tag->node_list_checksum = 0;
    
    // LoDs
    const char * const lods[] = {
        "superhigh",
        "high",
        "medium",
        "low",
        "superlow"
    };
    
    // Get our nodes
    std::vector<JMS::Node> nodes;
    
    // Sort JMSes into permutations
    std::map<std::string, JMSMap> permutations;
    std::string top_permutation;
    std::string top_lod;
    
    // Get regions
    std::vector<std::string> regions;
    
    for(auto &jms : map) {
        std::string lod = lods[0];
        std::string permutation = jms.first;
        
        // Find the string and LoD
        const auto *str = permutation.c_str();
        for(const auto *i = str; *i != 0; i++) {
            if(*i == ' ') {
                for(const auto *l : lods) {
                    if(std::strcmp(l, i + 1) == 0) {
                        lod = l;
                        permutation = std::string(str, i);
                        goto spaghetti_code_loop_done;
                    }
                }
            }
        }
        
        // Get the permutation map
        spaghetti_code_loop_done:
        auto &permutation_map = permutations[permutation];
        if(permutation_map.find(lod) != permutation_map.end()) {
            eprintf_error("Permutation %s has multiple %s LoDs", permutation.c_str(), lod.c_str());
            std::exit(EXIT_FAILURE);
        }
        permutation_map.emplace(lod, jms.second); // add it
        
        // Make sure it has nodes!
        if(jms.second.nodes.empty()) {
            eprintf_error("Permutation %s's %s LoD has no nodes", permutation.c_str(), lod.c_str());
            std::exit(EXIT_FAILURE);
        }
        
        // If we haven't added nodes, add them
        if(nodes.empty()) {
            nodes = jms.second.nodes;
            top_permutation = permutation;
            top_lod = lod;
        }
        
        // Otherwise, make sure we have the same nodes
        if(nodes != jms.second.nodes) {
            eprintf_error("Permutation %s's %s LoD does not match permutation %s's %s LoD's node", permutation.c_str(), lod.c_str(), top_permutation.c_str(), top_lod.c_str());
            std::exit(EXIT_FAILURE);
        }
        
        // Add any regions it may have
        for(auto &i : jms.second.regions) {
            auto iterator = regions.begin();
            bool has_it = false;
            for(; iterator != regions.end(); iterator++) {
                // The regions already exists
                if(i.name == *iterator) {
                    has_it = true;
                    break;
                }
                
                // This string value is less than it, so it goes before it
                else if(i.name < *iterator) {
                    break;
                }
            }
            
            // Add the new region if it doesn't exist
            if(!has_it) {
                regions.insert(iterator, i.name);
            }
        }
    }
    
    // Add nodes
    for(auto &n : nodes) {
        auto &node = model_tag->nodes.emplace_back();
        std::strncpy(node.name.string, n.name.c_str(), sizeof(node.name.string) - 1);
        node.next_sibling_node_index = n.sibling_node;
        node.first_child_node_index = n.first_child;
        node.default_translation = n.position;
        node.default_rotation = n.rotation;
        node.parent_node_index = NULL_INDEX;
    }
    
    // Bounds check the indices
    auto node_count = nodes.size();
    for(std::size_t n = 0; n < node_count; n++) {
        auto &node = nodes[n];
        if(node.first_child != NULL_INDEX && node.first_child >= node_count) {
            eprintf_error("Node %s has an out-of-bounds first child index", node.name.c_str());
            std::exit(EXIT_FAILURE);
        }
        if(node.sibling_node != NULL_INDEX && node.sibling_node >= node_count) {
            eprintf_error("Node %s has an out-of-bounds sibling node index", node.name.c_str());
            std::exit(EXIT_FAILURE);
        }
    }
    
    // Make sure we don't have infinite loops with sibling indices, too
    for(std::size_t n = 0; n < node_count; n++) {
        auto &node = nodes[n];
        
        std::size_t q = 0;
        if(node.sibling_node != NULL_INDEX) {
            std::size_t sibling;
            do {
                sibling = node.sibling_node;
                if(q++ > node_count) {
                    eprintf_error("Infinite loop detected with node %s's sibling index", node.name.c_str());
                    std::exit(EXIT_FAILURE);
                }
            } while(sibling != NULL_INDEX);
        }
    }
    
    // Set parent indices now
    for(std::size_t n = 0; n < node_count; n++) {
        auto &node = nodes[n];
        std::size_t next_node_to_set_parent = node.first_child;
        
        // Keep going until we hit null or something we already set
        while(next_node_to_set_parent != NULL_INDEX) {
            auto &child_node = model_tag->nodes[next_node_to_set_parent];
            
            // Did we already set a parent? Bail.
            if(child_node.parent_node_index != NULL_INDEX) {
                break;
            }
            
            // Calculate this
            child_node.node_distance_from_parent = child_node.default_translation.distance_from_point(HEK::Point3D<HEK::NativeEndian> {});
            child_node.parent_node_index = n;
            
            // Next node?
            next_node_to_set_parent = nodes[n].sibling_node;
        }
    }
    
    // Add regions to the model tag
    for(auto &i : regions) {
        auto &region = model_tag->regions.emplace_back();
        std::strncpy(region.name.string, i.c_str(), sizeof(region.name.string) - 1);
    }
    
    return tag->generate_hek_tag_data(fourcc);
}

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
    options.emplace_back("legacy", 'L', 0, "Use legacy behavior (use parent folder's filename for the tag name).");
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
            if(path.extension() == extension) {
                // Legacy - bump up a directory
                if(model_options.legacy) {
                    model_tag = path.parent_path();
                }
                
                // Otherwise use this
                else {
                    model_tag = path.replace_extension().string();
                }
            }
            else {
                eprintf_error("Extension must be %s", remaining_arguments[0]);
                return EXIT_FAILURE;
            }
        }
        else {
            auto model_folder_maybe = File::file_path_to_tag_path(remaining_arguments[0], model_options.data);
            if(model_folder_maybe.has_value() && std::filesystem::exists(remaining_arguments[0])) {
                model_tag = *model_folder_maybe;
            }
            else {
                eprintf_error("Failed to find a valid model %s in the data or tags directories.", remaining_arguments[0]);
                return EXIT_FAILURE;
            }
        }
    }
    else {
        model_tag = remaining_arguments[0];
    }
    
    // Double the filename if legacy
    auto data_dir = model_tag;
    if(model_options.legacy) {
        auto tp = std::filesystem::path(model_tag);
        model_tag = (tp / tp.filename()).string();
    }
    
    // Let's do this
    JMSMap jms_files;
    std::filesystem::path directory = model_options.data / data_dir / "models";
    
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
    
    // Generate a tag
    std::vector<std::byte> tag_data;
    std::filesystem::path file_path = (model_options.tags[0] / (model_tag + extension));
    
    switch(*model_options.type) {
        case ModelType::MODEL_TYPE_MODEL:
            tag_data = make_model_tag<Parser::Model, TagFourCC::TAG_FOURCC_MODEL>(file_path, model_options.tags, jms_files);
            break;
        case ModelType::MODEL_TYPE_GBXMODEL:
            tag_data = make_model_tag<Parser::GBXModel, TagFourCC::TAG_FOURCC_GBXMODEL>(file_path, model_options.tags, jms_files);
            break;
        default:
            std::terminate();
    }
    
    if(!File::save_file(file_path, tag_data)) {
        eprintf_error("Failed to write to %s", file_path.string().c_str());
        return EXIT_FAILURE;
    }
}
