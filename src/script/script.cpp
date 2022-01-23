// SPDX-License-Identifier: GPL-3.0-only

#include <map>
#include <cstdlib>
#include <cassert>
#include <filesystem>
#include <invader/printf.hpp>
#include <invader/command_line_option.hpp>
#include <invader/version.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/parser/parser.hpp>

#include <riat/riat.hpp>

int main(int argc, const char **argv) {
    using namespace Invader;
    //using namespace Invader::HEK;

    struct ScriptOption {
        const char *path;
        std::filesystem::path data = "data";
        std::filesystem::path tags = "tags";
    } script_options;
    script_options.path = *argv;

    // Add our options
    std::vector<CommandLineOption> options;
    options.emplace_back("info", 'i', 0);
    options.emplace_back("data", 'd', 1);
    options.emplace_back("tags", 't', 1);

    static constexpr char DESCRIPTION[] = "Compile scripts.";
    static constexpr char USAGE[] = "[options] <scenario>";

    // Parse arguments
    auto remaining_options = CommandLineOption::parse_arguments<ScriptOption &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, script_options, [](char opt, const auto &arguments, ScriptOption &script_options) {
        switch(opt) {
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);

            case 'd':
                script_options.data = arguments[0];
                break;

            case 't':
                script_options.tags = arguments[0];
                break;
        }
    });

    // Get the scenario tag
    std::string scenario;
    if(remaining_options.size() == 0) {
        eprintf("Expected a scenario tag. Use -h for more information.\n");
        return EXIT_FAILURE;
    }
    else if(remaining_options.size() > 1) {
        eprintf("Unexpected argument %s\n", remaining_options[1]);
        return EXIT_FAILURE;
    }
    else {
        scenario = File::halo_path_to_preferred_path(remaining_options[0]);
    }

    auto tag_path = script_options.tags / (scenario + ".scenario");
    auto script_directory_path = (script_options.data / scenario).parent_path() / "scripts";
    auto global_script_path = script_options.data / "global_scripts.hsc";
    
    // Open the scenario tag
    auto scenario_file_data = Invader::File::open_file(tag_path);
    if(!scenario_file_data.has_value()) {
        eprintf("Failed to open %s\n", tag_path.string().c_str());
        return EXIT_FAILURE;
    }

    // Parse it
    Parser::Scenario s;
    try {
        s = Parser::Scenario::parse_hek_tag_file((*scenario_file_data).data(), (*scenario_file_data).size());
    }
    catch(std::exception &e) {
        eprintf_error("Failed to parse %s: %s", tag_path.string().c_str(), e.what());
        return EXIT_FAILURE;
    }
    
    RIAT::Instance instance;
    
    decltype(s.source_files) source_files;
    auto load_script = [&source_files, &instance](const std::filesystem::path &path) {
        auto filename = path.filename();
        auto filename_without_extension = std::filesystem::path(filename).replace_extension().string();
        
        auto &source = source_files.emplace_back();
        
        oprintf("Loading %s...\n", filename.c_str());
        
        // Initialize
        source = {};
        
        // Check if it's too long. If not, copy. Otherwise, error
        if(filename_without_extension.size() > sizeof(source.name.string) - 1) {
            eprintf_error("Script file name '%s' is too long", filename_without_extension.c_str());
            throw std::exception();
        }
        std::strncpy(source.name.string, filename_without_extension.c_str(), sizeof(source.name.string) - 1);
        
        // Open it!
        auto f = File::open_file(path).value();
        instance.load_script_source(reinterpret_cast<const char *>(f.data()), f.size(), filename.string().c_str());
        
        // Move it into here
        source.source = std::move(f);
    };
    
    try {
        // First load the global scripts
        if(std::filesystem::exists(global_script_path)) {
            load_script(global_script_path);
        }
        
        // Next, load scripts in script directory
        std::list<std::filesystem::path> script_paths;
        
        if(std::filesystem::exists(script_directory_path)) {
            for(auto &p : std::filesystem::directory_iterator(script_directory_path)) {
                auto path = p.path();
                if(!p.is_regular_file() || path.extension() != ".hsc") {
                    continue;
                }
                script_paths.emplace_back(path);
            }
        }
        script_paths.sort();
        
        // Load in that order
        for(auto &path : script_paths) {
            load_script(path);
        }
        
        // Compile
        oprintf("Compiling all scripts...\n");
        instance.compile_scripts();
    }
    catch(std::exception &e) {
        eprintf_error("Failed to compile scripts: %s", e.what());
        throw std::exception();
    }
    
    std::size_t node_limit = 19001;
    
    auto scripts = instance.get_scripts();
    auto globals = instance.get_globals();
    auto nodes = instance.get_nodes();
    
    std::size_t node_count = nodes.size();
    
    if(nodes.size() > node_limit) {
        eprintf_error("Node limit exceeded for the target engine (%zu > %zu)", node_count, node_limit);
        throw std::exception();
    }
    
    std::vector<Invader::Parser::ScenarioScriptNode> into_nodes;
    
    auto format_index_to_id = [](std::size_t index) -> std::uint32_t {
        auto index_16_bit = static_cast<std::uint16_t>(index);
        return static_cast<std::uint32_t>(((index_16_bit + 0x6373) | 0x8000) << 16) | index_16_bit;
    };
    
    std::map<std::string, std::size_t> string_index;
    std::vector<std::byte> string_data;
    
    for(std::size_t node_index = 0; node_index < node_count; node_index++) {
        auto &n = nodes[node_index];
        auto &new_node = into_nodes.emplace_back();
        new_node = {};
        
        // Set the salt
        new_node.salt = format_index_to_id(node_index) >> 16;
        
        // If we have string data, add it
        if(n.string_data != NULL) {
            std::string str = n.string_data;
            if(!string_index.contains(str)) {
                string_index[str] = string_data.size();
                const auto *cstr = str.c_str();
                string_data.insert(string_data.end(), reinterpret_cast<const std::byte *>(cstr), reinterpret_cast<const std::byte *>(cstr) + str.size());
                string_data.emplace_back(std::byte());
            }
            new_node.string_offset = string_index[str];
        }
        
        // All nodes are marked with this...?
        new_node.flags |= Invader::HEK::ScenarioScriptNodeFlagsFlag::SCENARIO_SCRIPT_NODE_FLAGS_FLAG_IS_GARBAGE_COLLECTABLE;
        
        // Here's the type
        new_node.type = static_cast<Invader::HEK::ScenarioScriptValueType>(n.type);
        new_node.index_union = new_node.type;
        
        // Set this stuff
        if(n.is_primitive) {
            new_node.flags |= Invader::HEK::ScenarioScriptNodeFlagsFlag::SCENARIO_SCRIPT_NODE_FLAGS_FLAG_IS_PRIMITIVE;
            if(n.is_global) {
                new_node.flags |= Invader::HEK::ScenarioScriptNodeFlagsFlag::SCENARIO_SCRIPT_NODE_FLAGS_FLAG_IS_GLOBAL;
            }
            else {
                switch(n.type) {
                    case RIAT_ValueType::RIAT_VALUE_TYPE_BOOLEAN:
                        new_node.data.bool_int = n.bool_int;
                        break;
                    case RIAT_ValueType::RIAT_VALUE_TYPE_SCRIPT:
                    case RIAT_ValueType::RIAT_VALUE_TYPE_SHORT:
                        new_node.data.short_int = n.short_int;
                        break;
                    case RIAT_ValueType::RIAT_VALUE_TYPE_LONG:
                        new_node.data.long_int = n.long_int;
                        break;
                    case RIAT_ValueType::RIAT_VALUE_TYPE_REAL:
                        new_node.data.real = n.real;
                        break;
                    default:
                        break;
                }
            }
        }
        else {
            new_node.data.tag_id.id = format_index_to_id(n.child_node);
            
            if(n.is_script_call) {
                new_node.flags |= Invader::HEK::ScenarioScriptNodeFlagsFlag::SCENARIO_SCRIPT_NODE_FLAGS_FLAG_IS_SCRIPT_CALL;
                new_node.index_union = n.call_index;
            }
        }
        
        // Set the next node?
        if(n.next_node == SIZE_MAX) {
            new_node.next_node = UINT32_MAX;
        }
        else {
            new_node.next_node = format_index_to_id(n.next_node);
        }
    }
    
    using node_table_header_tag_fmt = Invader::Parser::ScenarioScriptNodeTable::struct_big;
    using node_tag_fmt = std::remove_reference<decltype(*into_nodes.data())>::type::struct_big;
    
    // Initialize the syntax data and write to it
    std::vector<std::byte> syntax_data(sizeof(node_table_header_tag_fmt) + node_limit * sizeof(node_tag_fmt));
    auto &table_output = *reinterpret_cast<node_table_header_tag_fmt *>(syntax_data.data());
    auto *node_output = reinterpret_cast<node_tag_fmt *>(&table_output + 1);
    table_output.count = node_count;
    table_output.size = node_count;
    table_output.maximum_count = node_limit;
    table_output.next_id = format_index_to_id(node_count) >> 16;
    table_output.element_size = sizeof(node_tag_fmt);
    table_output.data = 0x64407440;
    std::strcpy(table_output.name.string, "script node");
    table_output.one = 1;
    for(std::size_t node_index = 0; node_index < node_count; node_index++) {
        assert(sizeof(node_output[node_index]) == output.size());
        
        auto output = into_nodes[node_index].generate_hek_tag_data();
        memcpy(&node_output[node_index], output.data(), output.size());
    }
    
    std::size_t script_count = scripts.size();
    std::size_t global_count = globals.size();
    
    // Set up scripts
    decltype(s.scripts) new_scripts;
    new_scripts.resize(script_count);
    for(std::size_t s = 0; s < script_count; s++) {
        auto &new_script = new_scripts[s];
        const auto &cmp_script = scripts[s];
        
        static_assert(sizeof(new_script.name.string) == sizeof(cmp_script.name));
        memcpy(new_script.name.string, cmp_script.name, sizeof(cmp_script.name));
        
        new_script.return_type = static_cast<decltype(new_script.return_type)>(cmp_script.return_type);
        new_script.script_type = static_cast<decltype(new_script.script_type)>(cmp_script.script_type);
        new_script.root_expression_index = format_index_to_id(cmp_script.first_node);
    }
    
    // Set up globals
    decltype(s.globals) new_globals;
    new_globals.resize(global_count);
    for(std::size_t g = 0; g < global_count; g++) {
        auto &new_global = new_globals[g];
        const auto &cmp_global = globals[g];
        
        static_assert(sizeof(new_global.name.string) == sizeof(cmp_global.name));
        memcpy(new_global.name.string, cmp_global.name, sizeof(cmp_global.name));
        
        new_global.type = static_cast<decltype(new_global.type)>(cmp_global.value_type);
        new_global.initialization_expression_index = format_index_to_id(cmp_global.first_node);
    }
    
    string_data.resize(string_data.size() + 1024);
    
    // Clear out the script data
    s.scripts = std::move(new_scripts);
    s.globals = std::move(new_globals);
    s.source_files = std::move(source_files);
    s.script_string_data = std::move(string_data);
    s.script_syntax_data = std::move(syntax_data);
    
    // Write
    auto output = s.generate_hek_tag_data(Invader::HEK::TagFourCC::TAG_FOURCC_SCENARIO);
    if(File::save_file(tag_path, output)) {
        oprintf_success("Successfully compiled scripts");
        return EXIT_SUCCESS;
    }
    else {
        eprintf_error("Failed to write to %s", tag_path.string().c_str());
        return EXIT_FAILURE;
    }
}
