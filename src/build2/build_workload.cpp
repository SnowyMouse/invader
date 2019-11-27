// SPDX-License-Identifier: GPL-3.0-only

#include <invader/build2/build_workload.hpp>
#include <invader/hek/map.hpp>

namespace Invader {
    using namespace HEK;

    std::vector<std::byte> BuildWorkload2::compile_map (
        const char *scenario,
        const std::vector<std::string> &tags_directories,
        HEK::CacheFileEngine engine_target,
        std::string maps_directory,
        bool no_external_tags,
        bool always_index_tags,
        bool verbose,
        const std::optional<std::vector<std::tuple<TagClassInt, std::string>>> &with_index,
        const std::optional<std::uint32_t> &forge_crc,
        const std::optional<std::uint32_t> &tag_data_address,
        const std::optional<std::string> &rename_scenario
    ) {
        BuildWorkload2 workload;
        workload.scenario = scenario;
        workload.tags_directories = &tags_directories;

        // Set the tag data address
        if(tag_data_address.has_value()) {
            workload.tag_data_address = *tag_data_address;
        }
        else {
            switch(engine_target) {
                case CacheFileEngine::CACHE_FILE_DARK_CIRCLET:
                    workload.tag_data_address = CacheFileTagDataBaseMemoryAddress::CACHE_FILE_DARK_CIRCLET_BASE_MEMORY_ADDRESS;
                    break;
                case CacheFileEngine::CACHE_FILE_DEMO:
                    workload.tag_data_address = CacheFileTagDataBaseMemoryAddress::CACHE_FILE_DEMO_BASE_MEMORY_ADDRESS;
                    break;
                default:
                    workload.tag_data_address = CacheFileTagDataBaseMemoryAddress::CACHE_FILE_PC_BASE_MEMORY_ADDRESS;
                    break;
            }
        }

        auto return_value = workload.build_cache_file();
        return return_value;
    }

    std::vector<std::byte> BuildWorkload2::build_cache_file() {
        // Before we begin, add all of the tags
        this->add_tags();

        // Dedupe structs
        this->dedupe_structs();

        return std::vector<std::byte>();
    }

    std::size_t BuildWorkload2::compile_tag_recursively(const char *tag_path, TagClassInt tag_class_int) {
        // Search for the tag
        std::size_t return_value = this->tags.size();
        bool found = false;
        for(std::size_t i = 0; i < return_value; i++) {
            auto &tag = this->tags[i];
            if(tag.tag_class_int == tag_class_int && tag.path == tag_path) {
                if(tag.base_struct.has_value()) {
                    return i;
                }
                return_value = i;
                found = true;
                break;
            }
        }

        // If it wasn't found in the current array list, add it to the list and let's begin
        if(!found) {
            auto &tag = this->tags.emplace_back();
            tag.path = tag_path;
            tag.tag_class_int = tag_class_int;
        }

        std::terminate();

        return return_value;
    }

    void BuildWorkload2::add_tags() {
        this->scenario_index = this->compile_tag_recursively(this->scenario, TagClassInt::TAG_CLASS_SCENARIO);
        this->cache_file_type = reinterpret_cast<Scenario<LittleEndian> *>(this->structs[*this->tags[this->scenario_index].base_struct].data.data())->type;

        this->compile_tag_recursively("globals\\globals", TagClassInt::TAG_CLASS_GLOBALS);
        this->compile_tag_recursively("ui\\ui_tags_loaded_all_scenario_types", TagClassInt::TAG_CLASS_TAG_COLLECTION);

        // Load the correct tag collection tag
        switch(this->cache_file_type) {
            case ScenarioType::CACHE_FILE_SINGLEPLAYER:
                this->compile_tag_recursively("ui\\ui_tags_loaded_solo_scenario_type", TagClassInt::TAG_CLASS_TAG_COLLECTION);
                break;
            case ScenarioType::CACHE_FILE_MULTIPLAYER:
                this->compile_tag_recursively("ui\\ui_tags_loaded_multiplayer_scenario_type", TagClassInt::TAG_CLASS_TAG_COLLECTION);
                break;
            case ScenarioType::CACHE_FILE_USER_INTERFACE:
                this->compile_tag_recursively("ui\\ui_tags_loaded_mainmenu_scenario_type", TagClassInt::TAG_CLASS_TAG_COLLECTION);
                break;
        }

        // These are required for UI elements and other things
        this->compile_tag_recursively("sound\\sfx\\ui\\cursor", TagClassInt::TAG_CLASS_SOUND);
        this->compile_tag_recursively("sound\\sfx\\ui\\back", TagClassInt::TAG_CLASS_SOUND);
        this->compile_tag_recursively("sound\\sfx\\ui\\flag_failure", TagClassInt::TAG_CLASS_SOUND);
        this->compile_tag_recursively("ui\\shell\\main_menu\\mp_map_list", TagClassInt::TAG_CLASS_UNICODE_STRING_LIST);
        this->compile_tag_recursively("ui\\shell\\strings\\loading", TagClassInt::TAG_CLASS_UNICODE_STRING_LIST);
        this->compile_tag_recursively("ui\\shell\\bitmaps\\trouble_brewing", TagClassInt::TAG_CLASS_BITMAP);
        this->compile_tag_recursively("ui\\shell\\bitmaps\\background", TagClassInt::TAG_CLASS_BITMAP);
    }

    void BuildWorkload2::dedupe_structs() {
        bool found_something = true;
        while(true) {
            found_something = false;
            for(std::size_t i = 0; i < this->structs.size() && !found_something; i++) {
                for(std::size_t j = i + 1; j < this->structs.size(); j++) {
                    // Check if the structs are the same
                    if(this->structs[i] == this->structs[j]) {
                        // If so, go through every struct pointer. If they equal j, set to i. If they're greater than j, decrement
                        for(std::size_t k = 0; k < i; k++) {
                            for(auto &pointer : this->structs[k].pointers) {
                                if(pointer.struct_index > j) {
                                    (*pointer.struct_index)--;
                                }
                                else if(pointer.struct_index == j) {
                                    *pointer.struct_index = i;
                                }
                            }
                        }

                        this->structs.erase(this->structs.begin() + j);
                        found_something = true;
                        break;
                    }
                }
            }
        }
    }
}
