/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include <cctype>
#include <cmath>
#include <thread>

#ifndef NO_OUTPUT
#include <iostream>
#endif

#define BYTES_TO_MiB(bytes) ((bytes) / 1024.0 / 1024.0)

// This is the maximum length. 255 crashes Guerilla, and anything higher will not be loaded.
#define MAX_PATH_LENGTH 254

#include "../tag/hek/class/bitmap.hpp"
#include "../tag/hek/class/fog.hpp"
#include "../tag/hek/class/font.hpp"
#include "../tag/hek/class/gbxmodel.hpp"
#include "../tag/hek/class/particle.hpp"
#include "../tag/hek/class/scenario.hpp"
#include "../tag/hek/class/scenario_structure_bsp.hpp"
#include "../tag/hek/class/sound.hpp"
#include "../version.hpp"
#include "../error.hpp"
#include "../hek/map.hpp"

#include "build_workload.hpp"

namespace Invader {
    std::vector<std::byte> BuildWorkload::compile_map(
        std::string scenario,
        std::vector<std::string> tags_directories,
        std::string maps_directory,
        const std::vector<std::tuple<HEK::TagClassInt, std::string>> &with_index,
        bool indexed_tags,
        bool verbose
    ) {
        BuildWorkload workload;

        // First set up indexed tags
        workload.compiled_tags.reserve(with_index.size());
        for(auto &tag : with_index) {
            workload.compiled_tags.emplace_back(std::make_unique<CompiledTag>(std::get<1>(tag), std::get<0>(tag)));
        }

        // Add directory separator to the end of each directory
        std::vector<std::string> new_tag_dirs;
        new_tag_dirs.reserve(tags_directories.size());
        for(const auto &dir : tags_directories) {
            if(dir.size() == 0) {
                continue;
            }

            std::string new_dir = dir;
            #ifdef _WIN32
            if(new_dir[new_dir.size() - 1] != '\\' || new_dir[new_dir.size() - 1] != '/') {
                new_dir += '\\';
            }
            #else
            if(new_dir[new_dir.size() - 1] != '/') {
                new_dir += '/';
            }
            #endif

            new_tag_dirs.emplace_back(new_dir);
        }

        workload.tags_directories = new_tag_dirs;
        workload.maps_directory = maps_directory;
        if(indexed_tags && workload.maps_directory != "") {
            #ifdef _WIN32
            if(workload.maps_directory[workload.maps_directory.size() - 1] != '\\' || workload.maps_directory[workload.maps_directory.size() - 1] != '/') {
                workload.maps_directory += "\\";
            }
            #else
            if(workload.maps_directory[workload.maps_directory.size() - 1] != '/') {
                workload.maps_directory += "/";
            }
            #endif

            auto load_map = [](const std::string &path) -> std::vector<Resource> {
                std::FILE *f = std::fopen(path.data(), "rb");
                if(!f) {
                    #ifndef NO_OUTPUT
                    std::cerr << "Failed to open " << path << "\n";
                    #endif
                    return std::vector<Resource>();
                }
                std::fseek(f, 0, SEEK_END);
                std::size_t data_size = std::ftell(f);
                std::fseek(f, 0, SEEK_SET);
                std::unique_ptr<std::byte> data(new std::byte[data_size]);
                if(std::fread(data.get(), data_size, 1, f) != 1) {
                    #ifndef NO_OUTPUT
                    std::cerr << "Failed to open " << path << "\n";
                    #endif
                    return std::vector<Resource>();
                }
                return load_resource_map(data.get(), data_size);
            };
            workload.bitmaps = load_map(workload.maps_directory + "bitmaps.map");
            workload.sounds = load_map(workload.maps_directory + "sounds.map");
            workload.loc = load_map(workload.maps_directory + "loc.map");
        }
        workload.verbose = verbose;

        std::string scenario_backslash = scenario;
        for(std::size_t i = 0; i < scenario_backslash.length(); i++) {
            if(scenario_backslash[i] == '/') {
                scenario_backslash[i] = '\\';
            }
        }

        workload.scenario = scenario_backslash;

        return workload.build_cache_file();
    }

    std::vector<std::byte> BuildWorkload::build_cache_file() {
        using namespace HEK;

        // Get all the tags
        this->load_required_tags();
        this->tag_count = this->compiled_tags.size();
        if(this->tag_count > CACHE_FILE_MAX_TAG_COUNT) {
            std::cerr << "Tag count exceeds maximum of " << CACHE_FILE_MAX_TAG_COUNT << ".\n";
            throw MaximumTagDataSizeException();
        }

        // Remove anything we don't need
        this->index_tags();

        // Initialize our header and file data vector, also grabbing scenario information
        CacheFileHeader cache_file_header = {};
        std::vector<std::byte> file(sizeof(cache_file_header));
        std::strncpy(cache_file_header.name.string, get_scenario_name().data(), sizeof(cache_file_header.name.string) - 1);
        std::strncpy(cache_file_header.build.string, INVADER_VERSION_STRING, sizeof(cache_file_header.build.string));
        cache_file_header.map_type = this->cache_file_type;

        // eXoDux-specific bit
        bool x_dux = cache_file_header.map_type == 0x1004;

        // Start working on tag data
        std::vector<std::byte> tag_data(sizeof(CacheFileTagDataHeaderPC) + sizeof(CacheFileTagDataTag) * this->tag_count, std::byte());

        // Populate the tag array
        this->populate_tag_array(tag_data);

        // Add tag data
        this->add_tag_data(tag_data, file);

        // Get the tag data header
        auto &tag_data_header = *reinterpret_cast<CacheFileTagDataHeaderPC *>(tag_data.data());

        #ifndef NO_OUTPUT
        if(this->verbose) {
            std::printf("Scenario name:     %s\n", cache_file_header.name.string);

            std::size_t total_tag_size = 0;
            for(auto &tag : this->compiled_tags) {
                if(!tag->indexed) {
                    total_tag_size += tag->data_size;
                }
            }
            std::printf("Tags:              %zu / %zu (%.02f MiB)\n", compiled_tags.size(), CACHE_FILE_MAX_TAG_COUNT, BYTES_TO_MiB(total_tag_size));
        }
        #endif

        // Get the largest BSP tag
        std::size_t largest_bsp_size = 0;
        #ifndef NO_OUTPUT
        std::size_t largest_bsp = 0;
        #endif
        std::vector<std::size_t> bsps;
        std::size_t total_bsp_size = 0;
        std::size_t bsp_count = 0;
        for(std::size_t i = 0; i < this->tag_count; i++) {
            auto &tag = compiled_tags[i];
            if(tag->tag_class_int == TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP) {
                auto size = tag->data_size;
                total_bsp_size += size;
                bsp_count++;
                if(size > largest_bsp_size) {
                    largest_bsp_size = size;
                    #ifndef NO_OUTPUT
                    largest_bsp = i;
                    #endif
                }
                bsps.emplace_back(i);
            }
        }

        std::size_t max_tag_data_size = tag_data.size() + largest_bsp_size;

        // Output BSP info
        #ifndef NO_OUTPUT
        if(this->verbose) {
            std::printf("BSPs:              %zu (%.02f MiB)\n", bsp_count, BYTES_TO_MiB(total_bsp_size));
            for(auto bsp : bsps) {
                std::printf("                   %s (%.02f MiB)%s\n", compiled_tags[bsp]->path.data(), BYTES_TO_MiB(compiled_tags[bsp]->data_size), (bsp == largest_bsp) ? "*" : "");
            }
            std::printf("Tag space:         %.02f / %.02f MiB (%.02f %%)\n", BYTES_TO_MiB(max_tag_data_size), BYTES_TO_MiB(CACHE_FILE_MEMORY_LENGTH), max_tag_data_size * 100.0 / CACHE_FILE_MEMORY_LENGTH);
        }
        #endif

        // Check if we've exceeded the max amount of tag data
        if(max_tag_data_size > CACHE_FILE_MEMORY_LENGTH) {
            #ifndef NO_OUTPUT
            std::cerr << "Maximum tag data size exceeds budget.\n";
            #endif
            throw MaximumTagDataSizeException();
        }

        // Calculate approximate amount of data to reduce allocations needed
        std::size_t model_size = 0;
        std::size_t bitmap_sound_size = 0;
        for(auto &tag : compiled_tags) {
            auto asset_size = tag->asset_data.size();
            if(asset_size) {
                if(tag->tag_class_int == TagClassInt::TAG_CLASS_GBXMODEL || tag->tag_class_int == TagClassInt::TAG_CLASS_MODEL) {
                    model_size += asset_size;
                }
                else {
                    bitmap_sound_size += asset_size;
                }
            }
        }

        // Add model data
        std::vector<std::byte> vertices;
        std::vector<std::byte> indices;
        vertices.reserve(model_size);
        indices.reserve(model_size / 3);

        this->add_model_tag_data(vertices, indices, tag_data);
        auto model_data_size = vertices.size() + indices.size();

        #ifndef NO_OUTPUT
        if(this->verbose) {
            std::printf("Model data:        %.02f MiB\n", BYTES_TO_MiB(model_data_size));
        }
        #endif

        // Add bitmap and sound data
        file.reserve(file.size() + bitmap_sound_size + model_data_size + tag_data.size() + 4);
        this->add_bitmap_and_sound_data(file, tag_data);
        file.insert(file.end(), REQUIRED_PADDING_32_BIT(file.size()), std::byte());
        #ifndef NO_OUTPUT
        if(this->verbose) {
            std::size_t indexed_count = 0;

            for(auto &t : this->compiled_tags) {
                if(t->indexed) {
                    indexed_count++;
                }
            }

            std::printf("Bitmaps/sounds:    %.02f MiB\n", BYTES_TO_MiB(bitmap_sound_size));
            std::printf("Indexed tags:      %zu\n", indexed_count);
        }
        #endif

        // Get the size and offsets of model data
        auto model_data_offset = file.size();
        tag_data_header.vertex_size = static_cast<std::uint32_t>(vertices.size());
        tag_data_header.model_part_count_again = tag_data_header.model_part_count;
        tag_data_header.model_data_size = static_cast<std::uint32_t>(model_data_size);
        tag_data_header.model_data_file_offset = static_cast<std::uint32_t>(model_data_offset);
        tag_data_header.tags_literal = CACHE_FILE_TAGS;
        file.insert(file.end(), vertices.data(), vertices.data() + vertices.size());
        file.insert(file.end(), indices.data(), indices.data() + indices.size());
        file.insert(file.end(), REQUIRED_PADDING_32_BIT(file.size()), std::byte());
        vertices.clear();
        indices.clear();

        // Add tag data
        cache_file_header.tag_data_offset = static_cast<std::uint32_t>(file.size());
        file.insert(file.end(), tag_data.data(), tag_data.data() + tag_data.size());

        // Add the header
        cache_file_header.head_literal = CACHE_FILE_HEAD;
        cache_file_header.foot_literal = CACHE_FILE_FOOT;
        cache_file_header.tag_data_size = static_cast<std::uint32_t>(tag_data.size());
        cache_file_header.engine = CACHE_FILE_CUSTOM_EDITION;
        cache_file_header.file_size = static_cast<std::uint32_t>(file.size());
        cache_file_header.crc32 = 0x7E706156;
        std::snprintf(cache_file_header.build.string, sizeof(cache_file_header.build), "Invader " INVADER_VERSION_STRING);
        std::copy(reinterpret_cast<std::byte *>(&cache_file_header), reinterpret_cast<std::byte *>(&cache_file_header + 1), file.data());

        // Set eXoDux compatibility mode.
        if(x_dux) {
            auto *x_flag = reinterpret_cast<BigEndian<std::uint32_t> *>(file.data());
            for(std::size_t i = sizeof(cache_file_header) / sizeof(*x_flag); i < file.size() / sizeof(*x_flag); i++) {
                // Set compression rainbow bit
                auto flag = x_flag[i].read();
                auto flag2 = x_flag[i].read();
                flag |= (0b01010101010101010101010101010101 | 0b10101010101010101010101010101010);

                // XOR with magic number
                flag ^= !(flag2 & 0x1004) ? 0xAEAABEB4 : 0xB9B3BEAF;
                x_flag[i].write(flag);
            }
        }

        // Check if we've exceeded the maximum file size
        #ifndef NO_OUTPUT
        if(this->verbose) {
            std::printf("File size:         %.02f / %.02f MiB (%.02f %%)\n", BYTES_TO_MiB(file.size()), BYTES_TO_MiB(CACHE_FILE_MAXIMUM_FILE_LENGTH), file.size() * 100.0 / CACHE_FILE_MAXIMUM_FILE_LENGTH);
            if(file.size() > CACHE_FILE_MAXIMUM_FILE_LENGTH) {
                std::cerr << "Warning: File size exceeds Halo's limit. Map may require a mod to load.\n";
            }
        }
        #endif

        return file;
    }

    void BuildWorkload::index_tags() noexcept {
        using namespace HEK;

        for(auto &tag : this->compiled_tags) {
            switch(tag->tag_class_int) {
                case TagClassInt::TAG_CLASS_BITMAP:
                    for(std::size_t b = 0; b < this->bitmaps.size(); b+=2) {
                        if(this->bitmaps[b].data == tag->asset_data) {
                            tag->indexed = true;
                            tag->index = static_cast<std::uint32_t>(b + 1);
                            tag->asset_data.clear();
                            tag->data.clear();
                            break;
                        }
                    }
                    break;
                case TagClassInt::TAG_CLASS_SOUND:
                    for(std::size_t s = 0; s < this->sounds.size(); s+=2) {
                        if(this->sounds[s].data == tag->asset_data && this->sounds[s].name == tag->path + "__permutations") {
                            tag->indexed = true;
                            tag->asset_data.clear();
                            break;
                        }
                    }
                    break;
                case TagClassInt::TAG_CLASS_FONT:
                    for(std::size_t l = 0; l < this->loc.size(); l++) {
                        auto &loc_tag = this->loc[l];
                        if(loc_tag.name == tag->path) {
                            auto *tag_font = reinterpret_cast<Font<LittleEndian> *>(tag->data.data());
                            auto *loc_font = reinterpret_cast<Font<LittleEndian> *>(loc_tag.data.data());

                            std::size_t tag_font_pixel_size = tag_font->pixels.size;
                            std::size_t loc_font_pixel_size = loc_font->pixels.size;

                            // Make sure the size is the same
                            if(tag_font_pixel_size == loc_font_pixel_size) {
                                std::size_t tag_font_pixels = tag->resolve_pointer(&tag_font->pixels.pointer);
                                std::size_t loc_font_pixels = loc_font->pixels.pointer;

                                // And make sure that the pointer in the loc tag points to something
                                if(loc_font_pixels < loc_tag.data.size() && loc_font_pixels + loc_font_pixel_size <= loc_tag.data.size()) {
                                    auto *tag_font_pixel_data = tag->data.data() + tag_font_pixels;
                                    auto *loc_font_pixel_data = loc_tag.data.data() + loc_font_pixels;

                                    if(std::memcmp(tag_font_pixel_data, loc_font_pixel_data, tag_font_pixel_size) == 0) {
                                        tag->index = static_cast<std::uint32_t>(l);
                                        tag->indexed = true;
                                        tag->data.clear();
                                    }
                                }
                            }
                            break;
                        }
                    }
                    break;
                case TagClassInt::TAG_CLASS_UNICODE_STRING_LIST:
                    for(std::size_t l = 0; l < this->loc.size(); l++) {
                        auto &loc_tag = this->loc[l];
                        if(loc_tag.name == tag->path) {
                            // TODO: Compare strings.
                            if(loc_tag.data.size() == tag->data.size()) {
                                tag->index = static_cast<std::uint32_t>(l);
                                tag->indexed = true;
                                break;
                            }
                        }
                    }
                    break;
                case TagClassInt::TAG_CLASS_HUD_MESSAGE_TEXT:
                    for(std::size_t l = 0; l < this->loc.size(); l++) {
                        auto &loc_tag = this->loc[l];
                        if(loc_tag.name == tag->path) {
                            // TODO: Compare tag data.
                            if(loc_tag.data.size() == tag->data.size()) {
                                tag->index = static_cast<std::uint32_t>(l);
                                tag->indexed = true;
                                break;
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }

    void BuildWorkload::load_required_tags() {
        using namespace HEK;

        this->scenario_index = this->compile_tag_recursively(scenario, TagClassInt::TAG_CLASS_SCENARIO);
        this->cache_file_type = reinterpret_cast<Scenario<LittleEndian> *>(this->compiled_tags[this->scenario_index]->data.data())->type;
        this->compile_tag_recursively("globals\\globals", TagClassInt::TAG_CLASS_GLOBALS);
        this->compile_tag_recursively("ui\\ui_tags_loaded_all_scenario_types", TagClassInt::TAG_CLASS_TAG_COLLECTION);

        switch(this->cache_file_type) {
            case CacheFileType::CACHE_FILE_SINGLEPLAYER:
                this->compile_tag_recursively("ui\\ui_tags_loaded_solo_scenario_type", TagClassInt::TAG_CLASS_TAG_COLLECTION);
                break;
            case CacheFileType::CACHE_FILE_MULTIPLAYER:
                this->compile_tag_recursively("ui\\ui_tags_loaded_multiplayer_scenario_type", TagClassInt::TAG_CLASS_TAG_COLLECTION);
                break;
            case CacheFileType::CACHE_FILE_USER_INTERFACE:
                this->compile_tag_recursively("ui\\ui_tags_loaded_mainmenu_scenario_type", TagClassInt::TAG_CLASS_TAG_COLLECTION);
                break;
        }

        this->compile_tag_recursively("sound\\sfx\\ui\\cursor", TagClassInt::TAG_CLASS_SOUND);
        this->compile_tag_recursively("sound\\sfx\\ui\\back", TagClassInt::TAG_CLASS_SOUND);
        this->compile_tag_recursively("sound\\sfx\\ui\\flag_failure", TagClassInt::TAG_CLASS_SOUND);
        this->compile_tag_recursively("ui\\shell\\main_menu\\mp_map_list", TagClassInt::TAG_CLASS_UNICODE_STRING_LIST);
        this->compile_tag_recursively("ui\\shell\\strings\\loading", TagClassInt::TAG_CLASS_UNICODE_STRING_LIST);
        this->compile_tag_recursively("ui\\shell\\bitmaps\\trouble_brewing", TagClassInt::TAG_CLASS_BITMAP);
        this->compile_tag_recursively("ui\\shell\\bitmaps\\background", TagClassInt::TAG_CLASS_BITMAP);

        #ifndef NO_OUTPUT
        bool network_issue = false;
        #endif
        for(auto &compiled_tag : this->compiled_tags) {
            if(compiled_tag->stub()) {
                #ifndef NO_OUTPUT
                if(this->cache_file_type == CacheFileType::CACHE_FILE_MULTIPLAYER && (IS_OBJECT_TAG(compiled_tag->tag_class_int) || compiled_tag->tag_class_int == TagClassInt::TAG_CLASS_DAMAGE_EFFECT)) {
                    std::cerr << "Network object " << compiled_tag->path << "." << tag_class_to_extension(compiled_tag->tag_class_int) << " missing.\n";
                    network_issue = true;
                }
                #endif
                compiled_tag->path = std::string("stub\\") + tag_class_to_extension(compiled_tag->tag_class_int) + "\\" + compiled_tag->path;
                compiled_tag->tag_class_int = TagClassInt::TAG_CLASS_UNICODE_STRING_LIST;
                compiled_tag->data.insert(compiled_tag->data.begin(), 12, std::byte());
            }
        }

        #ifndef NO_OUTPUT
        if(network_issue) {
            std::cerr << "WARNING! The game WILL crash in multiplayer if missing tags are used.\n";
        }
        #endif
    }

    std::size_t BuildWorkload::compile_tag_recursively(const std::string &path, HEK::TagClassInt tag_class_int) {
        using namespace HEK;

        bool adding = true;
        std::size_t index;

        // First try to find the tag if it's not compiled
        for(std::size_t i = 0; i < compiled_tags.size(); i++) {
            auto &tag = this->compiled_tags[i];
            if(tag->tag_class_int == tag_class_int && tag->path == path) {
                if(tag->stub()) {
                    index = i;
                    adding = false;
                    break;
                }
                else {
                    return i;
                }
            }
        }

        if(adding) {
            index = this->compiled_tags.size();
        }

        // If it's a model tag, correct it to a gbxmodel
        if(tag_class_int == TagClassInt::TAG_CLASS_MODEL) {
            tag_class_int = TagClassInt::TAG_CLASS_GBXMODEL;
        }

        // Get the tag path, replacing all backslashes with forward slashes if not on Win32
        constexpr auto path_size = MAX_PATH_LENGTH + 1;
        if(this->tag_buffer.size() < path_size) {
            this->tag_buffer.resize(path_size);
        }
        char *tag_base_path = reinterpret_cast<char *>(this->tag_buffer.data());
        std::size_t actual_path_size = std::snprintf(tag_base_path, MAX_PATH_LENGTH, "%s.%s", path.data(), tag_class_to_extension(tag_class_int));
        if(actual_path_size >= path_size) {
            throw InvalidTagPathException();
        }

        #ifndef _WIN32
        for(std::size_t i = 0; i < actual_path_size; i++) {
            char &c = tag_base_path[i];
            if(c == '\\') {
                c = '/';
            }
        }
        #endif

        for(const auto &tag_dir : this->tags_directories) {
            // Concatenate the tag path
            std::string tag_path = tag_dir + tag_base_path;

            // Open the tag file
            std::FILE *file = std::fopen(tag_path.data(), "rb");
            if(!file) {
                continue;
            }

            std::fseek(file, 0, SEEK_END);
            std::size_t file_length = std::ftell(file);
            std::fseek(file, 0, SEEK_SET);

            if(file_length > this->tag_buffer.size()) {
                this->tag_buffer.resize(file_length, std::byte());
            }

            // Read the file
            if(std::fread(this->tag_buffer.data(), file_length, 1, file) != 1) {
                std::fclose(file);
                break;
            }

            // Close the file
            std::fclose(file);

            try {
                // Compile the tag
                std::unique_ptr<CompiledTag> tag(std::make_unique<CompiledTag>(path, tag_class_int, this->tag_buffer.data(), file_length, this->cache_file_type));
                CompiledTag *tag_ptr = tag.get();

                // Insert into the tag array
                if(adding) {
                    this->compiled_tags.emplace_back(std::move(tag));
                }
                else {
                    this->compiled_tags[index] = std::move(tag);
                }

                // Iterate through all of the tags this tag references
                for(auto &dependency : tag_ptr->dependencies) {
                    if(dependency.tag_class_int == TagClassInt::TAG_CLASS_MODEL) {
                        dependency.tag_class_int = TagClassInt::TAG_CLASS_GBXMODEL;
                    }
                    auto *dependency_in_tag = reinterpret_cast<TagDependency<LittleEndian> *>(tag_ptr->data.data() + dependency.offset);
                    dependency_in_tag->tag_id = tag_id_from_index(this->compile_tag_recursively(dependency.path, dependency.tag_class_int));
                    dependency_in_tag->tag_class_int = dependency.tag_class_int;
                }

                // BSP-related things (need to set water plane stuff for fog)
                if(tag_ptr->tag_class_int == HEK::TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP) {
                    auto *bsp_data = tag_ptr->data.data();
                    auto &bsp_header = *reinterpret_cast<ScenarioStructureBSPCompiledHeader *>(bsp_data);
                    std::size_t bsp_offset = tag_ptr->resolve_pointer(&bsp_header.pointer);
                    if(bsp_offset != INVALID_POINTER) {
                        auto &bsp = *reinterpret_cast<ScenarioStructureBSP<LittleEndian> *>(bsp_data + bsp_offset);

                        std::size_t fog_palette_offset = tag_ptr->resolve_pointer(&bsp.fog_palette.pointer);
                        std::size_t fog_region_offset = tag_ptr->resolve_pointer(&bsp.fog_regions.pointer);
                        std::size_t fog_plane_offset = tag_ptr->resolve_pointer(&bsp.fog_planes.pointer);

                        if(fog_palette_offset != INVALID_POINTER && fog_region_offset != INVALID_POINTER && fog_plane_offset != INVALID_POINTER) {
                            auto *fog_planes = reinterpret_cast<ScenarioStructureBSPFogPlane<LittleEndian> *>(bsp_data + fog_plane_offset);
                            auto *fog_regions = reinterpret_cast<ScenarioStructureBSPFogRegion<LittleEndian> *>(bsp_data + fog_region_offset);
                            auto *fog_palette = reinterpret_cast<ScenarioStructureBSPFogPalette<LittleEndian> *>(bsp_data + fog_palette_offset);

                            std::size_t fog_plane_count = bsp.fog_planes.count;
                            std::size_t fog_region_count = bsp.fog_regions.count;
                            std::size_t fog_palette_count = bsp.fog_palette.count;

                            // Go through each fog plane
                            for(std::size_t i = 0; i < fog_plane_count; i++) {
                                auto &plane = fog_planes[i];

                                // Find what region this fog is in
                                std::size_t region_index = plane.front_region;
                                if(region_index > fog_region_count) {
                                    continue;
                                }
                                auto &region = fog_regions[region_index];

                                // Lastly get what fog tag
                                std::size_t palette_index = region.fog_palette;
                                if(palette_index > fog_palette_count) {
                                    continue;
                                }
                                const auto &fog_id = fog_palette[palette_index].fog.tag_id.read();
                                if(fog_id.id == 0xFFFFFFFF) {
                                    continue;
                                }
                                auto &fog_tag = this->compiled_tags[fog_id.index];
                                if(fog_tag->tag_class_int != TagClassInt::TAG_CLASS_FOG) {
                                    continue;
                                }

                                auto *fog = reinterpret_cast<Fog<LittleEndian> *>(fog_tag->data.data());
                                if(fog->flags.read().is_water) {
                                    plane.material_type = MaterialType::MATERIAL_TYPE_WATER;
                                }
                            }
                        }
                    }
                }

                // Particle-related things
                else if(tag_ptr->tag_class_int == HEK::TagClassInt::TAG_CLASS_PARTICLE) {
                    auto &particle = *reinterpret_cast<Particle<LittleEndian> *>(tag_ptr->data.data());
                    if(particle.bitmap.tag_id.read().id == 0xFFFFFFFF) {
                        #ifndef NO_OUTPUT
                        std::cerr << tag_ptr->path << ".particle has no bitmap.\n";
                        #endif
                        throw;
                    }
                    else {
                        auto &bitmap_tag = this->compiled_tags[particle.bitmap.tag_id.read().index];
                        auto &bitmap = *reinterpret_cast<Bitmap<LittleEndian> *>(bitmap_tag->data.data());

                        // Calculating this value requires looking at the bitmap's sprite(s)
                        particle.unknown = 1.0f / static_cast<float>(std::pow(static_cast<float>(2.0f), static_cast<float>(bitmap.sprite_budget_size.read())) * 32.0f); // 1/32 if 32x32, 1/64 if 64x64, etc.

                        auto bitmap_data_offset = bitmap_tag->resolve_pointer(&bitmap.bitmap_data.pointer);
                        std::vector<std::int16_t> widths(bitmap.bitmap_data.count);
                        std::vector<std::int16_t> heights(bitmap.bitmap_data.count);
                        if(bitmap_data_offset != INVALID_POINTER) {
                            for(std::size_t i = 0; i < bitmap.bitmap_data.count; i++) {
                                auto &bitmap_data = reinterpret_cast<BitmapData<LittleEndian> *>(bitmap_tag->data.data() + bitmap_data_offset)[i];
                                widths[i] = bitmap_data.width;
                                heights[i] = bitmap_data.height;
                            }
                        }

                        auto sequence_offset = bitmap_tag->resolve_pointer(&bitmap.bitmap_group_sequence.pointer);
                        if(sequence_offset != INVALID_POINTER) {
                            float max_difference = 0.0f;
                            for(std::size_t sequence_index = 0; sequence_index < bitmap.bitmap_group_sequence.count; sequence_index++) {
                                auto &sequence = reinterpret_cast<BitmapGroupSequence<LittleEndian> *>(bitmap_tag->data.data() + sequence_offset)[sequence_index];
                                auto first_sprite_offset = bitmap_tag->resolve_pointer(&sequence.sprites.pointer);

                                // We'll need to iterate through all of the sprites
                                std::size_t sprite_count = sequence.sprites.count;
                                if(first_sprite_offset != INVALID_POINTER) {
                                    for(std::size_t i = 0; i < sprite_count; i++) {
                                        auto &sprite = reinterpret_cast<BitmapGroupSprite<LittleEndian> *>(bitmap_tag->data.data() + first_sprite_offset)[i];

                                        if(static_cast<std::size_t>(sprite.bitmap_index) > widths.size()) {
                                            continue;
                                        }

                                        float difference_a = (sprite.right - sprite.left) * widths[sprite.bitmap_index];
                                        float difference_b = (sprite.bottom - sprite.top) * heights[sprite.bitmap_index];

                                        if(difference_a > max_difference) {
                                            max_difference = difference_a;
                                        }
                                        if(difference_b > max_difference) {
                                            max_difference = difference_b;
                                        }
                                    }
                                }
                            }

                            // Divide by the maximum length (or width) we got out of all of the sprites
                            particle.unknown = 1.0f / max_difference;
                        }
                    }
                }

                // Scenario-related things
                else if(tag_ptr->tag_class_int == TagClassInt::TAG_CLASS_SCENARIO) {
                    auto *scenario_data = tag_ptr->data.data();
                    auto &scenario = *reinterpret_cast<Scenario<LittleEndian> *>(scenario_data);

                    // Get decals
                    std::size_t decal_offset = tag_ptr->resolve_pointer(&scenario.decals.pointer);
                    std::size_t decal_count = scenario.decals.count;
                    std::vector<ScenarioStructureBSPRuntimeDecal<LittleEndian>> decals;
                    if(decal_offset != INVALID_POINTER && decal_count > 0) {
                        auto *decals_ptr = reinterpret_cast<ScenarioDecal<LittleEndian> *>(scenario_data + decal_offset);
                        for(std::size_t i = 0; i < decal_count; i++) {
                            ScenarioStructureBSPRuntimeDecal<LittleEndian> copy = {};
                            copy.position = decals_ptr[i].position;
                            copy.decal_type = decals_ptr[i].decal_type;
                            copy.yaw = decals_ptr[i].yaw;
                            copy.pitch = decals_ptr[i].pitch;
                            decals.push_back(copy);
                        }
                    }

                    // Add all the decals to all the bsps all the time
                    std::size_t bsp_offset = tag_ptr->resolve_pointer(&scenario.structure_bsps.pointer);
                    std::size_t bsp_count = scenario.structure_bsps.count;
                    if(bsp_offset != INVALID_POINTER && decal_count > 0) {
                        auto *bsps = reinterpret_cast<ScenarioBSP<LittleEndian> *>(scenario_data + bsp_offset);
                        for(std::size_t bsp = 0; bsp < bsp_count; bsp++) {
                            auto bsp_id = bsps[bsp].structure_bsp.tag_id.read().index;
                            if(bsp_id < this->compiled_tags.size()) {
                                auto &bsp_tag = this->compiled_tags[bsp_id];
                                auto *bsp_header = reinterpret_cast<ScenarioStructureBSPCompiledHeader *>(bsp_tag->data.data());
                                std::size_t bsp_data_offset = bsp_tag->resolve_pointer(&bsp_header->pointer);
                                if(bsp_data_offset == INVALID_POINTER) {
                                    continue;
                                }

                                // Allocate runtime decal space
                                auto *bsp_data = reinterpret_cast<ScenarioStructureBSP<LittleEndian> *>(bsp_tag->data.data() + bsp_data_offset);
                                std::vector<ScenarioStructureBSPRuntimeDecal<LittleEndian>> runtime_decals;

                                // Make sure we know which decals we used for this BSP so we don't reuse them
                                std::vector<bool> used(decals.size(), false);
                                std::size_t bsp_cluster_offset = bsp_tag->resolve_pointer(&bsp_data->clusters.pointer);
                                std::size_t bsp_cluster_count = bsp_data->clusters.count;
                                if(bsp_cluster_offset == INVALID_POINTER) {
                                    continue;
                                }

                                // Get clusters
                                auto *clusters = reinterpret_cast<ScenarioStructureBSPCluster<LittleEndian> *>(bsp_tag->data.data() + bsp_cluster_offset);
                                for(std::size_t c = 0; c < bsp_cluster_count; c++) {
                                    auto &cluster = clusters[c];
                                    std::vector possible(decal_count, false);
                                    std::size_t subcluster_offset = bsp_tag->resolve_pointer(&cluster.subclusters.pointer);
                                    std::size_t subcluster_count = cluster.subclusters.count;
                                    auto *subclusters = reinterpret_cast<ScenarioStructureBSPSubcluster<LittleEndian> *>(bsp_tag->data.data() + subcluster_offset);
                                    if(subcluster_offset == INVALID_POINTER) {
                                        continue;
                                    }

                                    // Go through subclusters to see which one can hold a decal
                                    for(std::size_t s = 0; s < subcluster_count; s++) {
                                        auto &subcluster = subclusters[s];
                                        for(std::size_t d = 0; d < decal_count; d++) {
                                            if(used[d]) {
                                                continue;
                                            }
                                            auto &decal = decals[d];
                                            float x = decal.position.x;
                                            float y = decal.position.y;
                                            float z = decal.position.z;
                                            if(x >= subcluster.world_bounds_x.from && x <= subcluster.world_bounds_x.to &&
                                               y >= subcluster.world_bounds_y.from && y <= subcluster.world_bounds_y.to &&
                                               z >= subcluster.world_bounds_z.from && z <= subcluster.world_bounds_z.to) {
                                                possible[d] = true;
                                            }
                                        }
                                    }

                                    // Now add all of the decals that we found
                                    std::size_t runtime_decal_count_first = runtime_decals.size();
                                    std::size_t cluster_decal_count = 0;

                                    for(std::size_t p = 0; p < decal_count; p++) {
                                        if(possible[p]) {
                                            runtime_decals.push_back(decals[p]);
                                            cluster_decal_count++;
                                            used[p] = true;
                                        }
                                    }

                                    // Set the decal count
                                    if(cluster_decal_count != 0) {
                                        cluster.first_decal_index = static_cast<std::int16_t>(runtime_decal_count_first);
                                        cluster.decal_count = static_cast<std::int16_t>(cluster_decal_count);
                                    }
                                    else {
                                        cluster.first_decal_index = -1;
                                        cluster.decal_count = 0;
                                    }
                                }

                                // Set the decal count to the number of decals we used and add it to the end of the BSP
                                bsp_data->runtime_decals.count = runtime_decals.size();

                                if(runtime_decals.size() != 0) {
                                    std::size_t bsp_runtime_decal_offset = reinterpret_cast<std::byte *>(&bsp_data->runtime_decals.pointer) - bsp_tag->data.data();
                                    bsp_tag->pointers.push_back(CompiledTagPointer { bsp_runtime_decal_offset, bsp_tag->data.size() });
                                    bsp_tag->data.insert(bsp_tag->data.end(), reinterpret_cast<std::byte *>(runtime_decals.data()), reinterpret_cast<std::byte *>(runtime_decals.data() + runtime_decals.size()));
                                }
                            }
                        }
                    }
                }

                // If we need predicted resources, let's get them
                else if(IS_OBJECT_TAG(tag_ptr->tag_class_int)) {
                    // Set this in case we get cyclical references (references that directly or indirectly reference themself)
                    std::vector<bool> tags_read(this->compiled_tags.size(), false);

                    // Here are our dependencies
                    std::vector<HEK::PredictedResource<LittleEndian>> predicted_resources;

                    auto &compiled_tags_ref = this->compiled_tags;
                    auto recursively_read = [&predicted_resources, &tags_read, &compiled_tags_ref](std::size_t tag, auto &recursion) {
                        if(tag >= compiled_tags_ref.size()) {
                            throw OutOfBoundsException();
                        }

                        if(tags_read[tag]) {
                            return;
                        }

                        tags_read[tag] = true;

                        auto &tag_reference = compiled_tags_ref[tag];
                        auto tag_class = tag_reference->tag_class_int;
                        if(IS_OBJECT_TAG(tag_class)) {
                            return;
                        }
                        else if(tag_class == TAG_CLASS_SOUND || tag_class == TAG_CLASS_BITMAP) {
                            HEK::PredictedResource<LittleEndian> resource = {};
                            resource.tag = tag_id_from_index(tag);
                            resource.type = tag_class == TAG_CLASS_BITMAP ? PredictedResourceType::PREDICTED_RESOUCE_TYPE_BITMAP : PredictedResourceType::PREDICTED_RESOUCE_TYPE_SOUND;
                            predicted_resources.push_back(resource);
                        }
                        else {
                            for(auto &dependency : tag_reference->dependencies) {
                                auto *dependency_in_tag = reinterpret_cast<TagDependency<LittleEndian> *>(tag_reference->data.data() + dependency.offset);
                                recursion(dependency_in_tag->tag_id.read().index, recursion);
                            }
                        }
                    };

                    if(IS_OBJECT_TAG(tag_ptr->tag_class_int)) {
                        for(auto &dependency : tag_ptr->dependencies) {
                            auto *dependency_in_tag = reinterpret_cast<TagDependency<LittleEndian> *>(tag_ptr->data.data() + dependency.offset);
                            if(dependency_in_tag->tag_class_int == TagClassInt::TAG_CLASS_MODEL ||
                                dependency_in_tag->tag_class_int == TagClassInt::TAG_CLASS_GBXMODEL) {
                                recursively_read(dependency_in_tag->tag_id.read().index, recursively_read);
                            }
                        }
                    }

                    // Add our predicted resources, somehow
                    std::size_t size_of_resources = predicted_resources.size() * sizeof(*predicted_resources.data());
                    if(IS_OBJECT_TAG(tag_ptr->tag_class_int)) {
                        // Find where we want to add the pointer
                        std::size_t offset_to_add = tag_ptr->data.size();
                        for(std::size_t p = tag_ptr->pointers.size() - 1; p < tag_ptr->pointers.size(); p--) {
                            auto &ptr = tag_ptr->pointers[p];
                            if(ptr.offset < 0x170) {
                                break;
                            }
                            else {
                                offset_to_add = ptr.offset_pointed;
                            }
                        }

                        // Offset everything that is after where we're adding data
                        for(auto &ptr : tag_ptr->pointers) {
                            if(ptr.offset >= offset_to_add) {
                                ptr.offset += size_of_resources;
                            }
                            if(ptr.offset_pointed >= offset_to_add) {
                                ptr.offset_pointed += size_of_resources;
                            }
                        }
                        for(auto &dep : tag_ptr->dependencies) {
                            if(dep.offset >= offset_to_add) {
                                dep.offset += size_of_resources;
                            }
                        }

                        // Insert data
                        auto *begin = reinterpret_cast<const std::byte *>(predicted_resources.data());
                        tag_ptr->data.insert(tag_ptr->data.begin() + offset_to_add, begin, begin + size_of_resources);

                        // Apply offsets
                        auto *resource_reference = reinterpret_cast<TagReflexive<LittleEndian, PredictedResource> *>(tag_ptr->data.data() + 0x170);
                        resource_reference->count = static_cast<std::uint32_t>(predicted_resources.size());

                        // Add offset (in order)
                        bool add_to_end = true;
                        CompiledTagPointer ptr_to_add = { 0x170 + 0x4, offset_to_add };
                        for(std::size_t p = 0; p < tag_ptr->pointers.size(); p++) {
                            auto &ptr = tag_ptr->pointers[p];
                            if(ptr.offset >= offset_to_add) {
                                tag_ptr->pointers.insert(tag_ptr->pointers.begin() + p, ptr_to_add);
                                add_to_end = false;
                                break;
                            }
                        }
                        if(add_to_end) {
                            tag_ptr->pointers.push_back(ptr_to_add);
                        }
                    }
                }

                return index;
            }
            catch(...) {
                #ifndef NO_OUTPUT
                std::cerr << "Failed to compile " << path << "." << tag_class_to_extension(tag_class_int) << "\n";
                #endif
                throw;
            }
        }

        #ifndef NO_OUTPUT
        std::cerr << "Could not find " << path << "." << tag_class_to_extension(tag_class_int) << "\n";
        #endif
        throw FailedToOpenTagException();
    }

    std::string BuildWorkload::get_scenario_name() {
        std::string map_name = this->scenario;
        for(const char *map_name_i = this->scenario.data(); *map_name_i; map_name_i++) {
            if(*map_name_i == '\\' || *map_name_i == '/') {
                map_name = map_name_i + 1;
            }
        }

        // Get name length
        std::size_t map_name_length = map_name.length();

        // Error if greater than 31 characters.
        if(map_name_length > 31) {
            #ifndef NO_OUTPUT
            std::cerr << "Scenario name `" << map_name << "` exceeds 31 characters.\n";
            #endif
            throw InvalidScenarioNameException();
        }

        // Copy, erroring if a capital letter is detected
        for(std::size_t i = 0; i < map_name_length; i++) {
            char character = map_name[i];
            char lowercase = std::tolower(character);
            if(character != lowercase) {
                #ifndef NO_OUTPUT
                std::cerr << "Scenario name `" << map_name << "` contains a capital letter.\n";
                #endif
                throw InvalidScenarioNameException();
            }
        }

        return map_name;
    }

    struct DedupingAssetData {
        std::size_t offset;
        std::size_t size;
    };

    void BuildWorkload::populate_tag_array(std::vector<std::byte> &tag_data) {
        using namespace HEK;

        // Set parts of the header
        auto &tag_data_header = *reinterpret_cast<CacheFileTagDataHeaderPC *>(tag_data.data());
        tag_data_header.scenario_tag = tag_id_from_index(this->scenario_index);
        reinterpret_cast<CacheFileTagDataHeaderPC *>(tag_data.data())->tag_array_address = this->tag_data_address + sizeof(tag_data_header);
        tag_data_header.tag_count = static_cast<std::uint32_t>(this->tag_count);

        std::vector<DedupingAssetData> deduping_tag_paths;
        deduping_tag_paths.reserve(this->compiled_tags.size());

        // Do tag paths and the tag array
        for(std::size_t i = 0; i < this->tag_count; i++) {
            auto &tag_data_tag = reinterpret_cast<CacheFileTagDataTag *>(tag_data.data() + sizeof(tag_data_header))[i];
            auto &compiled_tag = this->compiled_tags[i];

            // Write the tag class and the class(es) inherited (if applicable). This isn't necessary as the map will work just fine, but tool.exe does it for some reason.
            tag_data_tag.primary_class = compiled_tag->tag_class_int;
            tag_data_tag.secondary_class = TagClassInt::TAG_CLASS_NONE;
            tag_data_tag.tertiary_class = TagClassInt::TAG_CLASS_NONE;
            switch(tag_data_tag.primary_class) {
                case TagClassInt::TAG_CLASS_SHADER_ENVIRONMENT:
                case TagClassInt::TAG_CLASS_SHADER_MODEL:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_CHICAGO:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_CHICAGO_EXTENDED:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GENERIC:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_GLASS:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_METER:
                case TagClassInt::TAG_CLASS_SHADER_TRANSPARENT_PLASMA:
                    tag_data_tag.secondary_class = TagClassInt::TAG_CLASS_SHADER;
                    break;
                case TagClassInt::TAG_CLASS_PLACEHOLDER:
                case TagClassInt::TAG_CLASS_SCENERY:
                case TagClassInt::TAG_CLASS_SOUND_SCENERY:
                case TagClassInt::TAG_CLASS_PROJECTILE:
                    tag_data_tag.secondary_class = TagClassInt::TAG_CLASS_OBJECT;
                    break;
                case TagClassInt::TAG_CLASS_BIPED:
                case TagClassInt::TAG_CLASS_UNIT:
                    tag_data_tag.secondary_class = TagClassInt::TAG_CLASS_UNIT;
                    tag_data_tag.tertiary_class = TagClassInt::TAG_CLASS_OBJECT;
                    break;
                case TagClassInt::TAG_CLASS_EQUIPMENT:
                case TagClassInt::TAG_CLASS_WEAPON:
                case TagClassInt::TAG_CLASS_GARBAGE:
                    tag_data_tag.secondary_class = TagClassInt::TAG_CLASS_ITEM;
                    tag_data_tag.tertiary_class = TagClassInt::TAG_CLASS_OBJECT;
                    break;
                case TagClassInt::TAG_CLASS_DEVICE_MACHINE:
                case TagClassInt::TAG_CLASS_DEVICE_LIGHT_FIXTURE:
                case TagClassInt::TAG_CLASS_DEVICE_CONTROL:
                    tag_data_tag.secondary_class = TagClassInt::TAG_CLASS_DEVICE;
                    tag_data_tag.tertiary_class = TagClassInt::TAG_CLASS_OBJECT;
                    break;
                default:
                    break;
            }

            // Write the tag ID
            tag_data_tag.tag_id = tag_id_from_index(i);

            // Check to see if we already have the tag path written
            bool deduped = false;
            std::size_t path_length = compiled_tag->path.length();
            for(auto &duped_path : deduping_tag_paths) {
                if(duped_path.size == path_length && std::memcmp(tag_data.data() + duped_path.offset, compiled_tag->path.data(), path_length) == 0) {
                    tag_data_tag.tag_path = static_cast<Pointer>(this->tag_data_address + duped_path.offset);
                    deduped = true;
                    break;
                }
            }

            // Write the tag path if not
            if(!deduped) {
                std::size_t offset = tag_data.size();
                deduping_tag_paths.push_back(DedupingAssetData { offset, path_length });
                tag_data_tag.tag_path = static_cast<Pointer>(this->tag_data_address + offset);
                const auto *compiled_tag_path = reinterpret_cast<std::byte *>(compiled_tag->path.data());
                tag_data.insert(tag_data.end(), compiled_tag_path, compiled_tag_path + path_length);
                tag_data.insert(tag_data.end(), std::byte());
            }
        }

        // Add any required padding to 32-bit align it
        tag_data.insert(tag_data.end(), REQUIRED_PADDING_32_BIT(tag_data.size()), std::byte());
    }

    void BuildWorkload::add_tag_data(std::vector<std::byte> &tag_data, std::vector<std::byte> &file) {
        using namespace HEK;

        auto *tag_array_ptr = reinterpret_cast<CacheFileTagDataTag *>(tag_data.data() + sizeof(CacheFileTagDataHeaderPC));
        std::vector<CacheFileTagDataTag> tag_array(tag_array_ptr, tag_array_ptr + this->tag_count);

        for(std::size_t i = 0; i < this->tag_count; i++) {
            auto &compiled_tag = this->compiled_tags[i];

            // If indexed, deal with it appropriately
            if(compiled_tag->indexed) {
                tag_array[i].indexed = 1;
                if(compiled_tag->tag_class_int != TagClassInt::TAG_CLASS_SOUND) {
                    tag_array[i].tag_data = compiled_tag->index;
                    continue;
                }
            }

            // Skip BSP tags
            if(compiled_tag->tag_class_int == TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP) {
                continue;
            }

            // Write tag data
            auto offset = this->add_tag_data_for_tag(tag_data, tag_array.data(), i);
            tag_array[i].tag_data = static_cast<std::uint32_t>(this->tag_data_address + offset);

            // Go through all BSPs if scenario tag
            if(compiled_tag->tag_class_int == TagClassInt::TAG_CLASS_SCENARIO) {
                auto *scenario_tag = reinterpret_cast<Scenario<LittleEndian> *>(tag_data.data() + offset);

                // Get the BSPs
                auto *bsps = scenario_tag->structure_bsps.get_structs(tag_data, this->tag_data_address);
                std::size_t bsp_count = scenario_tag->structure_bsps.count;
                for(std::size_t bsp = 0; bsp < bsp_count; bsp++) {
                    auto &bsp_struct = bsps[bsp];

                    // Check if it's a valid reference
                    auto bsp_index = bsp_struct.structure_bsp.tag_id.read().index;
                    if(bsp_index >= this->compiled_tags.size()) {
                        #ifndef NO_OUTPUT
                        std::cerr << "Invalid BSP reference in scenario tag\n";
                        #endif
                        throw InvalidDependencyException();
                    }

                    // Check if it's a BSP tag
                    auto &bsp_compiled_tag = this->compiled_tags[bsp_index];
                    if(bsp_compiled_tag->tag_class_int != TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP) {
                        #ifndef NO_OUTPUT
                        std::cerr << "Mismatched BSP reference in scenario tag\n";
                        #endif
                        throw InvalidDependencyException();
                    }

                    // Add it
                    bsp_struct.bsp_size = static_cast<std::uint32_t>(bsp_compiled_tag->data.size());
                    bsp_struct.bsp_address = this->tag_data_address + CACHE_FILE_MEMORY_LENGTH - bsp_struct.bsp_size;
                    bsp_struct.bsp_start = static_cast<std::uint32_t>(add_tag_data_for_tag(file, tag_array.data(), bsp_index));
                    bsp_compiled_tag->data.clear();
                }
            }

            // Free data that's no longer in use
            compiled_tag->data.clear();
        }

        std::copy(tag_array.data(), tag_array.data() + this->tag_count, reinterpret_cast<CacheFileTagDataTag *>(tag_data.data() + sizeof(CacheFileTagDataHeaderPC)));
    }

    std::size_t BuildWorkload::add_tag_data_for_tag(std::vector<std::byte> &tag_data, void *tag_array, std::size_t tag) {
        using namespace HEK;

        auto &compiled_tag = this->compiled_tags[tag];
        compiled_tag->data_size = compiled_tag->data.size();
        auto offset = tag_data.size();
        tag_data.insert(tag_data.end(), compiled_tag->data.data(), compiled_tag->data.data() + compiled_tag->data.size());
        auto *tag_data_data = tag_data.data() + offset;
        auto *tag_array_cast = reinterpret_cast<CacheFileTagDataTag *>(tag_array);

        // Adjust for all pointers
        for(auto &pointer : compiled_tag->pointers) {
            if(pointer.offset + sizeof(std::uint32_t) > compiled_tag->data.size() || pointer.offset_pointed > compiled_tag->data.size()) {
                std::cerr << "Invalid pointer for " << compiled_tag->path << "." << tag_class_to_extension(compiled_tag->tag_class_int) << "\n";
                throw InvalidPointerException();
            }
            std::uint32_t new_offset;
            if(compiled_tag->tag_class_int == TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP) {
                new_offset = static_cast<std::uint32_t>(this->tag_data_address + CACHE_FILE_MEMORY_LENGTH - compiled_tag->data.size() + pointer.offset_pointed);
            }
            else {
                new_offset = static_cast<std::uint32_t>(this->tag_data_address + offset + pointer.offset_pointed);
            }
            *reinterpret_cast<std::uint32_t *>(tag_data_data + pointer.offset) = new_offset;
        }

        // Adjust for all dependencies
        for(auto &dependency : compiled_tag->dependencies) {
            if(dependency.offset + sizeof(TagDependency<LittleEndian>) > compiled_tag->data.size()) {
                std::cerr << "Invalid dependency offset for " << compiled_tag->path << "." << tag_class_to_extension(compiled_tag->tag_class_int) << "\n";
                throw InvalidDependencyException();
            }
            auto &dependency_data = *reinterpret_cast<TagDependency<LittleEndian> *>(tag_data_data + dependency.offset);

            // Resolve the dependency
            std::size_t depended_tag_id = dependency_data.tag_id.read().index;
            if(depended_tag_id >= this->tag_count) {
                std::cerr << "Invalid dependency index for " << compiled_tag->path << "." << tag_class_to_extension(compiled_tag->tag_class_int) << "\n";
                throw InvalidDependencyException();
            }

            dependency_data.path_pointer = tag_array_cast[depended_tag_id].tag_path;
            dependency_data.tag_class_int = tag_array_cast[depended_tag_id].primary_class;
            dependency_data.path_size = 0;
            dependency_data.tag_id = tag_array_cast[depended_tag_id].tag_id;
        }

        // Add any required padding to 32-bit align it
        tag_data.insert(tag_data.end(), REQUIRED_PADDING_32_BIT(tag_data.size()), std::byte());

        return offset;
    }

    void BuildWorkload::add_bitmap_and_sound_data(std::vector<std::byte> &file, std::vector<std::byte> &tag_data) {
        using namespace HEK;

        std::vector<DedupingAssetData> all_asset_data;
        auto &tag_data_header = *reinterpret_cast<CacheFileTagDataHeaderPC *>(tag_data.data());
        auto *tags = reinterpret_cast<CacheFileTagDataTag *>(tag_data.data() + sizeof(tag_data_header));
        for(std::size_t i = 0; i < this->tag_count; i++) {
            if(this->compiled_tags[i]->indexed) {
                continue;
            }

            auto &tag = tags[i];
            std::size_t tag_asset_data_size = this->compiled_tags[i]->asset_data.size();
            auto tag_class = tag.primary_class.read();
            if(tag_class == TagClassInt::TAG_CLASS_GBXMODEL || tag_class == TagClassInt::TAG_CLASS_MODEL) {
                continue;
            }

            // Get the file offset
            auto *tag_asset_data = this->compiled_tags[i]->asset_data.data();

            // Check what tag we're dealing with
            switch(tag_class) {
                case TagClassInt::TAG_CLASS_BITMAP: {
                    auto &bitmap_tag_data = *reinterpret_cast<Bitmap<LittleEndian> *>(tag_data.data() + (tag.tag_data - this->tag_data_address));
                    std::size_t bitmaps_count = bitmap_tag_data.bitmap_data.count;
                    auto *bitmaps_data = bitmap_tag_data.bitmap_data.get_structs(tag_data, this->tag_data_address);

                    // Get the offsets of each bitmap, making sure each offset is valid
                    std::vector<std::size_t> offsets(bitmaps_count);
                    for(std::size_t b = 0; b < bitmaps_count; b++) {
                        std::size_t pixels_offset = bitmaps_data[b].pixels_offset;
                        if(pixels_offset > tag_asset_data_size) {
                            std::cerr << "Invalid pixels offset for bitmap " << b << " for " << this->compiled_tags[i]->path << "." << tag_class_to_extension(this->compiled_tags[i]->tag_class_int) << "\n";
                            throw OutOfBoundsException();
                        }
                        offsets[b] = pixels_offset;
                    }

                    // Calculate the sizes of each bitmap
                    std::vector<std::size_t> sizes(bitmaps_count);
                    for(std::size_t b = 0; b < bitmaps_count; b++) {
                        std::size_t size = tag_asset_data_size - offsets[b];
                        for(std::size_t b2 = 0; b2 < bitmaps_count; b2++) {
                            if(offsets[b2] > offsets[b]) {
                                std::size_t potential_size = offsets[b2] - offsets[b];
                                if(potential_size < size) {
                                    size = potential_size;
                                }
                            }
                        }
                        sizes[b] = size;
                    }

                    // Write the data
                    for(std::size_t b = 0; b < bitmaps_count; b++) {
                        bitmaps_data[b].pixels_count = static_cast<std::int32_t>(sizes[b]);

                        // Make sure it's not duplicate
                        bool duped = false;
                        for(auto &asset : all_asset_data) {
                            if(asset.size == sizes[b] && std::memcmp(tag_asset_data + offsets[b], file.data() + asset.offset, asset.size) == 0) {
                                bitmaps_data[b].pixels_offset = static_cast<std::int32_t>(asset.offset);
                                duped = true;
                                break;
                            }
                        }

                        if(!duped) {
                            bitmaps_data[b].pixels_offset = static_cast<std::int32_t>(file.size());
                            all_asset_data.push_back(DedupingAssetData { file.size(), sizes[b] });
                            file.insert(file.end(), tag_asset_data + offsets[b], tag_asset_data + offsets[b] + sizes[b]);
                        }

                        bitmaps_data[b].bitmap_class = tag_class;
                        bitmaps_data[b].bitmap_tag_id = tag.tag_id;
                    }

                    break;
                }
                case TagClassInt::TAG_CLASS_SOUND: {
                    auto &sound_tag_data = *reinterpret_cast<Sound<LittleEndian> *>(tag_data.data() + (tag.tag_data - this->tag_data_address));
                    std::size_t pitch_range_count = sound_tag_data.pitch_ranges.count;
                    auto *pitch_range_data = sound_tag_data.pitch_ranges.get_structs(tag_data, this->tag_data_address);
                    for(std::size_t p = 0; p < pitch_range_count; p++) {
                        auto &pitch_range = pitch_range_data[p];
                        std::size_t permutation_count = pitch_range.permutations.count;
                        auto *permutation_data = pitch_range.permutations.get_structs(tag_data, this->tag_data_address);

                        // Write the data
                        for(std::size_t r = 0; r < permutation_count; r++) {
                            auto &permutation = permutation_data[r];
                            std::size_t size = permutation.samples.size;
                            std::size_t offset = permutation.samples.file_offset;

                            // Make sure it's not duplicate
                            bool duped = false;
                            for(auto &asset : all_asset_data) {
                                if(asset.size == size && std::memcmp(tag_asset_data + offset, file.data() + asset.offset, asset.size) == 0) {
                                    permutation.samples.file_offset = static_cast<std::int32_t>(asset.offset);
                                    duped = true;
                                    break;
                                }
                            }

                            if(!duped) {
                                permutation.samples.file_offset = static_cast<std::int32_t>(file.size());
                                all_asset_data.push_back(DedupingAssetData { file.size(), size });
                                file.insert(file.end(), tag_asset_data + offset, tag_asset_data + offset + size);
                            }

                            permutation.tag_id_0 = tag.tag_id;
                            permutation.tag_id_1 = tag.tag_id;
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    struct DedupingIndexData {
        std::vector<std::byte> data;
        std::vector<HEK::GBXModelGeometryPart<HEK::LittleEndian> *> parts;
    };

    void BuildWorkload::add_model_tag_data(std::vector<std::byte> &vertices, std::vector<std::byte> &indices, std::vector<std::byte> &tag_data) {
        using namespace HEK;

        auto &tag_data_header = *reinterpret_cast<CacheFileTagDataHeaderPC *>(tag_data.data());
        auto *tags = reinterpret_cast<CacheFileTagDataTag *>(tag_data.data() + sizeof(tag_data_header));

        std::vector<DedupingAssetData> deduping_vertices;
        std::vector<DedupingIndexData> deduping_indices;

        for(std::size_t i = 0; i < this->tag_count; i++) {
            auto &tag = tags[i];
            if(tag.primary_class != TagClassInt::TAG_CLASS_GBXMODEL) {
                continue;
            }

            // Get the model tag
            auto *model_data = this->compiled_tags[i]->asset_data.data();
            auto model_data_size = this->compiled_tags[i]->asset_data.size();
            auto &model_tag_data = *reinterpret_cast<GBXModel<LittleEndian> *>(tag_data.data() + (tag.tag_data - this->tag_data_address));
            std::size_t geometry_count = model_tag_data.geometries.count;
            auto *geometry_data = model_tag_data.geometries.get_structs(tag_data, this->tag_data_address);

            // Iterate through geometries
            for(std::size_t g = 0; g < geometry_count; g++) {
                auto &geometry = geometry_data[g];
                std::size_t parts_count = geometry.parts.count;
                auto *parts_data = geometry.parts.get_structs(tag_data, this->tag_data_address);
                tag_data_header.model_part_count = static_cast<std::uint32_t>(tag_data_header.model_part_count + parts_count);

                // Iterate through parts
                for(std::size_t p = 0; p < parts_count; p++) {
                    auto &part = parts_data[p];

                    auto vertex_offset = part.vertex_offset;
                    auto *part_vertices = model_data + part.vertex_offset;

                    auto index_offset = part.triangle_offset;
                    auto *part_indices = model_data + part.triangle_offset;

                    std::size_t vertex_size = part.vertex_count * sizeof(GBXModelVertexUncompressed<LittleEndian>);
                    if(vertex_size + vertex_offset > model_data_size) {
                        std::cerr << "Invalid vertex size for part " << g << " - " << p << " for " << this->compiled_tags[i]->path << "." << tag_class_to_extension(this->compiled_tags[i]->tag_class_int) << "\n";
                        throw OutOfBoundsException();
                    }

                    std::size_t index_size = (part.triangle_count + 2) * sizeof(std::uint16_t);
                    if(index_size + index_offset > model_data_size) {
                        std::cerr << "Invalid index size for part " << g << " - " << p << " for " << this->compiled_tags[i]->path << "." << tag_class_to_extension(this->compiled_tags[i]->tag_class_int) << "\n";
                        throw OutOfBoundsException();
                    }

                    // Check if indices are duplicated
                    bool duped_indices = false;
                    for(auto &duped_index : deduping_indices) {
                        std::size_t duped_size = duped_index.data.size();

                        // Check if this matches the top of something we already found
                        if(duped_size >= index_size && std::memcmp(part_indices, duped_index.data.data(), index_size) == 0) {
                            duped_index.parts.push_back(&part);
                            duped_indices = true;
                            break;
                        }

                        // Check if what we already found matches the beginning. If so, we can expand it
                        else if(duped_size < index_size && std::memcmp(part_indices, duped_index.data.data(), duped_size) == 0) {
                            duped_index.parts.push_back(&part);
                            duped_index.data.insert(duped_index.data.end(), part_indices + duped_size / sizeof(*part_indices), part_indices + index_size / sizeof(*part_indices));
                            duped_indices = true;
                            break;
                        }
                    }

                    if(!duped_indices) {
                        DedupingIndexData data;
                        data.data.insert(data.data.end(), part_indices, part_indices + index_size / sizeof(*part_indices));
                        data.parts.push_back(&part);
                        deduping_indices.push_back(std::move(data));
                    }

                    // Check if vertices are duplicated
                    bool duped_vertices = false;
                    for(auto &duped_part : deduping_vertices) {
                        if(duped_part.size == vertex_size && std::memcmp(part_vertices, vertices.data() + duped_part.offset, vertex_size) == 0) {
                            part.vertex_offset = static_cast<std::uint32_t>(duped_part.offset);
                            duped_vertices = true;
                            break;
                        }
                    }

                    if(!duped_vertices) {
                        part.vertex_offset = static_cast<std::uint32_t>(vertices.size());
                        deduping_vertices.push_back(DedupingAssetData {vertices.size(), vertex_size});
                        vertices.insert(vertices.end(), part_vertices, part_vertices + vertex_size / sizeof(*part_vertices));
                    }
                }
            }

            // Free data no longer being used.
            this->compiled_tags[i]->asset_data.clear();
        }

        // Add all of the unique indices
        for(auto &duped_index : deduping_indices) {
            std::size_t offset = indices.size();
            indices.insert(indices.end(), duped_index.data.data(), duped_index.data.data() + duped_index.data.size());
            for(auto *part : duped_index.parts) {
                part->triangle_offset = static_cast<std::uint32_t>(offset);
                part->triangle_offset_2 = part->triangle_offset;
            }
        }
    }
}
