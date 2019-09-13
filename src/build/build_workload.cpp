/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include <cctype>
#include <cmath>
#include <thread>

#include "../resource/list/resource_list.hpp"

#define BYTES_TO_MiB(bytes) ((bytes) / 1024.0 / 1024.0)

// This is the maximum length. 255 crashes Guerilla, and anything higher will not be loaded.
#define MAX_PATH_LENGTH 254

#include "../eprintf.hpp"
#include "../tag/hek/class/biped.hpp"
#include "../tag/hek/class/bitmap.hpp"
#include "../tag/hek/class/detail_object_collection.hpp"
#include "../tag/hek/class/fog.hpp"
#include "../tag/hek/class/font.hpp"
#include "../tag/hek/class/gbxmodel.hpp"
#include "../tag/hek/class/particle.hpp"
#include "../tag/hek/class/scenario.hpp"
#include "../tag/hek/class/scenario_structure_bsp.hpp"
#include "../tag/hek/class/weather_particle_system.hpp"
#include "../tag/hek/class/sound.hpp"
#include "../tag/hek/class/string_list.hpp"
#include "../version.hpp"
#include "../error.hpp"
#include "../hek/map.hpp"
#include "../crc/hek/crc.hpp"

#include "build_workload.hpp"

namespace Invader {
    std::vector<std::byte> BuildWorkload::compile_map(
        const char *scenario,
        std::vector<std::string> tags_directories,
        std::string maps_directory,
        const std::vector<std::tuple<HEK::TagClassInt, std::string>> &with_index,
        bool no_indexed_tags,
        bool always_index_tags,
        bool verbose,
        const std::uint32_t *forge_crc
    ) {
        BuildWorkload workload;

        workload.always_index_tags = always_index_tags;

        // First set up indexed tags
        workload.compiled_tags.reserve(with_index.size());
        for(auto &tag : with_index) {
            workload.compiled_tags.emplace_back(std::make_unique<CompiledTag>(std::get<1>(tag), std::get<0>(tag)));
        }

        // Add directory separator to the end of each directory
        std::vector<std::string> new_tag_dirs;
        new_tag_dirs.reserve(tags_directories.size());
        for(const auto &dir : tags_directories) {
            // Skip empty paths
            if(dir.size() == 0) {
                continue;
            }

            // End with a directory separator
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

            // Add to the tags directory list
            new_tag_dirs.emplace_back(new_dir);
        }

        // Load resource maps if we need to do so
        workload.tags_directories = new_tag_dirs;
        workload.maps_directory = maps_directory;
        if(!no_indexed_tags && workload.maps_directory != "") {
            // End with a directory separator if not already done so
            #ifdef _WIN32
            if(workload.maps_directory[workload.maps_directory.size() - 1] != '\\' || workload.maps_directory[workload.maps_directory.size() - 1] != '/') {
                workload.maps_directory += "\\";
            }
            #else
            if(workload.maps_directory[workload.maps_directory.size() - 1] != '/') {
                workload.maps_directory += "/";
            }
            #endif

            std::vector<std::byte> resource_data_buffer;

            // Load all resource maps
            auto load_map = [&resource_data_buffer](const std::string &path) -> std::vector<Resource> {
                std::FILE *f = std::fopen(path.data(), "rb");
                if(!f) {
                    eprintf("Failed to open %s\n", path.data());
                    return std::vector<Resource>();
                }
                std::fseek(f, 0, SEEK_END);
                std::size_t data_size = std::ftell(f);
                std::fseek(f, 0, SEEK_SET);

                if(data_size > resource_data_buffer.size()) {
                    resource_data_buffer.resize(data_size);
                }

                if(std::fread(resource_data_buffer.data(), data_size, 1, f) != 1) {
                    eprintf("Failed to open %s\n", path.data());
                    std::fclose(f);
                    return std::vector<Resource>();
                }

                std::fclose(f);
                return load_resource_map(resource_data_buffer.data(), data_size);
            };
            workload.bitmaps = load_map(workload.maps_directory + "bitmaps.map");
            workload.sounds = load_map(workload.maps_directory + "sounds.map");
            workload.loc = load_map(workload.maps_directory + "loc.map");
        }
        workload.verbose = verbose;

        // Replace forward slashes in scenario tag path with backslashes
        workload.scenario = scenario;
        for(char &c : workload.scenario) {
            if(c == '/') {
                c = '\\';
            }
        }

        return workload.build_cache_file(forge_crc);
    }

    std::vector<std::byte> BuildWorkload::build_cache_file(const std::uint32_t *forge_crc) {
        using namespace HEK;

        // Get all the tags
        this->load_required_tags();
        this->tag_count = this->compiled_tags.size();
        if(this->tag_count > CACHE_FILE_MAX_TAG_COUNT) {
            eprintf("Tag count exceeds maximum of %zu.\n", CACHE_FILE_MAX_TAG_COUNT);
            throw MaximumTagDataSizeException();
        }

        // Remove anything we don't need
        std::size_t total_indexed_data = this->index_tags();

        // Initialize our header and file data vector, also grabbing scenario information
        CacheFileHeader cache_file_header = {};
        std::vector<std::byte> file(sizeof(cache_file_header));
        std::strncpy(cache_file_header.name.string, get_scenario_name().data(), sizeof(cache_file_header.name.string) - 1);
        cache_file_header.map_type = this->cache_file_type;

        // eXoDux-specific bit
        bool x_dux = cache_file_header.map_type == 0x1004;

        // Start working on tag data
        std::vector<std::byte> tag_data(sizeof(CacheFileTagDataHeaderPC) + sizeof(CacheFileTagDataTag) * this->tag_count, std::byte());

        // Populate the tag array
        this->populate_tag_array(tag_data);

        // Fix the scenario tag
        this->fix_scenario_tag_encounters();
        this->fix_scenario_tag_command_lists();

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

        // Get the largest BSP tag as well as usage of the indexed tag space
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

        std::size_t max_tag_data_size = tag_data.size() + largest_bsp_size + total_indexed_data;

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
            eprintf("Maximum tag data size exceeds budget.\n");
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
        cache_file_header.file_size = 0; // do NOT set file size; this breaks Halo!
        std::snprintf(cache_file_header.build.string, sizeof(cache_file_header.build), INVADER_FULL_VERSION_STRING);
        std::copy(reinterpret_cast<std::byte *>(&cache_file_header), reinterpret_cast<std::byte *>(&cache_file_header + 1), file.data());

        // Get the CRC and set it in the header
        CacheFileHeader &cache_file_header_file = *reinterpret_cast<CacheFileHeader *>(file.data());
        if(forge_crc) {
            std::uint32_t new_random;
            cache_file_header_file.crc32 = calculate_map_crc(file.data(), file.size(), forge_crc, &new_random);
            CacheFileTagDataHeader &header = *reinterpret_cast<CacheFileTagDataHeader *>(file.data() + cache_file_header.tag_data_offset);
            header.random_number = new_random;
        }
        else {
            cache_file_header_file.crc32 = calculate_map_crc(file.data(), file.size());
        }

        #ifndef NO_OUTPUT
        if(this->verbose) {
            std::printf("CRC32 checksum:    0x%08X\n", cache_file_header_file.crc32.read());
        }
        #endif

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

        // Output the file size
        #ifndef NO_OUTPUT
        if(this->verbose) {
            std::printf("File size:         %.02f MiB / %.02f MiB (%.02f %%)\n", BYTES_TO_MiB(file.size()), BYTES_TO_MiB(CACHE_FILE_MAXIMUM_FILE_LENGTH), file.size() * 100.0F / CACHE_FILE_MAXIMUM_FILE_LENGTH);
        }
        #endif

        // Check if we've exceeded the max file size.
        if(file.size() > CACHE_FILE_MAXIMUM_FILE_LENGTH) {
            eprintf("Maximum file size exceeds budget.\n");
            throw MaximumFileSizeException();
        }

        return file;
    }

    std::size_t BuildWorkload::index_tags() noexcept {
        using namespace HEK;

        // Get the amount of data removed
        std::size_t total_removed_data = 0;

        // If we're always indexing tags when possible, match by path
        if(this->always_index_tags) {
            for(auto &tag : this->compiled_tags) {
                switch(tag->tag_class_int) {
                    case TagClassInt::TAG_CLASS_BITMAP:
                        for(std::size_t b = 1; b < this->bitmaps.size(); b+=2) {
                            if(this->bitmaps[b].name == tag->path) {
                                total_removed_data += tag->data.size();
                                tag->indexed = true;
                                tag->index = static_cast<std::uint32_t>(b);
                                tag->asset_data.clear();
                                tag->data.clear();
                                break;
                            }
                        }
                        break;
                    case TagClassInt::TAG_CLASS_SOUND:
                        for(std::size_t s = 1; s < this->sounds.size(); s+=2) {
                            if(this->sounds[s].name == tag->path) {
                                tag->indexed = true;
                                tag->asset_data.clear();
                                break;
                            }
                        }
                        break;
                    case TagClassInt::TAG_CLASS_FONT:
                    case TagClassInt::TAG_CLASS_UNICODE_STRING_LIST:
                    case TagClassInt::TAG_CLASS_HUD_MESSAGE_TEXT:
                        for(std::size_t l = 0; l < this->loc.size(); l++) {
                            if(this->loc[l].name == tag->path) {
                                tag->indexed = true;
                                tag->data.clear();
                                tag->index = static_cast<std::uint32_t>(l);
                                break;
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        // Otherwise, do this:
        else {
            for(auto &tag : this->compiled_tags) {
                switch(tag->tag_class_int) {
                    case TagClassInt::TAG_CLASS_BITMAP:
                        for(std::size_t b = 0; b < this->bitmaps.size(); b+=2) {
                            if(this->bitmaps[b].data == tag->asset_data) {
                                tag->indexed = true;
                                tag->index = static_cast<std::uint32_t>(b + 1);
                                tag->asset_data.clear();
                                total_removed_data += this->bitmaps[b + 1].data.size();
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
                                            total_removed_data += loc_tag.data.size();
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
                                auto *tag_base = tag->data.data();
                                auto *loc_base = loc_tag.data.data();

                                auto *tag_list = reinterpret_cast<StringList<LittleEndian> *>(tag_base);
                                auto *loc_list = reinterpret_cast<StringList<LittleEndian> *>(loc_base);
                                std::uint32_t string_count = tag_list->strings.count.read();

                                // First check if they have the same number of strings
                                bool removed = false;
                                if(loc_list->strings.count == string_count) {
                                    // Next, iterate through each string and compare them
                                    auto *tag_strings = reinterpret_cast<HEK::StringListString<LittleEndian> *>(tag_base + tag->resolve_pointer(&tag_list->strings.pointer));
                                    auto *loc_strings = reinterpret_cast<HEK::StringListString<LittleEndian> *>(loc_base + loc_list->strings.pointer.read());

                                    for(std::uint32_t s = 0; s < string_count; s++) {
                                        auto *tag_str = reinterpret_cast<LittleEndian<std::uint16_t> *>(tag_base + tag->resolve_pointer(&tag_strings[s].string.pointer));
                                        auto *loc_str = reinterpret_cast<LittleEndian<std::uint16_t> *>(loc_base + loc_strings[s].string.pointer.read());

                                        // Iterate through each character until they aren't equal anymore or we hit a null byte
                                        while(*tag_str != 0 && *loc_str != 0 && *tag_str == *loc_str) {
                                            tag_str++;
                                            loc_str++;
                                        }

                                        removed = *tag_str != *loc_str;
                                        if(removed) {
                                            break;
                                        }
                                    }
                                }
                                else {
                                    removed = true;
                                }

                                // If we had to remove it, give up
                                if(!removed) {
                                    tag->index = static_cast<std::uint32_t>(l);
                                    tag->indexed = true;
                                    total_removed_data += loc_tag.data.size();
                                    tag->data.clear();
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
                                    total_removed_data += loc_tag.data.size();
                                    tag->data.clear();
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

        return total_removed_data;
    }

    void BuildWorkload::load_required_tags() {
        using namespace HEK;

        this->scenario_index = this->compile_tag_recursively(this->scenario.data(), TagClassInt::TAG_CLASS_SCENARIO);
        this->cache_file_type = reinterpret_cast<Scenario<LittleEndian> *>(this->compiled_tags[this->scenario_index]->data.data())->type;
        auto scenario_name = this->get_scenario_name();

        // Depending on the type of map we're building, use a certain resource limit
        std::size_t bitmap_limit, sound_limit, loc_limit;

        // Singleplayer / UI limits - use the internal list
        if(this->cache_file_type != CacheFileType::CACHE_FILE_MULTIPLAYER) {
            bitmap_limit = get_default_bitmap_resources_count();
            sound_limit = get_default_sound_resources_count();
            loc_limit = get_default_loc_resources_count();
        }

        // Multiplayer limits
        else {
            bitmap_limit = 853;
            sound_limit = 376;
            loc_limit = 176;
        }

        bitmap_limit *= 2;
        sound_limit *= 2;

        while(this->bitmaps.size() > bitmap_limit) {
            this->bitmaps.erase(this->bitmaps.begin() + this->bitmaps.size() - 1);
        }
        while(this->sounds.size() > sound_limit) {
            this->sounds.erase(this->sounds.begin() + this->sounds.size() - 1);
        }
        while(this->loc.size() > loc_limit) {
            this->loc.erase(this->loc.begin() + this->loc.size() - 1);
        }

        // We'll need to load these tags for all map types
        this->compile_tag_recursively("globals\\globals", TagClassInt::TAG_CLASS_GLOBALS);
        this->compile_tag_recursively("ui\\ui_tags_loaded_all_scenario_types", TagClassInt::TAG_CLASS_TAG_COLLECTION);

        // Load the correct tag collection tag
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

        // These are required for UI elements and other things
        this->compile_tag_recursively("sound\\sfx\\ui\\cursor", TagClassInt::TAG_CLASS_SOUND);
        this->compile_tag_recursively("sound\\sfx\\ui\\back", TagClassInt::TAG_CLASS_SOUND);
        this->compile_tag_recursively("sound\\sfx\\ui\\flag_failure", TagClassInt::TAG_CLASS_SOUND);
        this->compile_tag_recursively("ui\\shell\\main_menu\\mp_map_list", TagClassInt::TAG_CLASS_UNICODE_STRING_LIST);
        this->compile_tag_recursively("ui\\shell\\strings\\loading", TagClassInt::TAG_CLASS_UNICODE_STRING_LIST);
        this->compile_tag_recursively("ui\\shell\\bitmaps\\trouble_brewing", TagClassInt::TAG_CLASS_BITMAP);
        this->compile_tag_recursively("ui\\shell\\bitmaps\\background", TagClassInt::TAG_CLASS_BITMAP);

        // If we're using an indexed list of tags to maintain the same tag order as another map, check to make sure bad things won't happen when using this map due to missing tags.
        #ifndef NO_OUTPUT
        bool network_issue = false;
        #endif
        for(auto &compiled_tag : this->compiled_tags) {
            if(compiled_tag->stub()) {
                // Damage effects and object tags that are not in the correct location will break things
                #ifndef NO_OUTPUT
                if(this->cache_file_type == CacheFileType::CACHE_FILE_MULTIPLAYER && (IS_OBJECT_TAG(compiled_tag->tag_class_int) || compiled_tag->tag_class_int == TagClassInt::TAG_CLASS_DAMAGE_EFFECT)) {
                    eprintf("Warning: Network object %s.%s is missing.\n", compiled_tag->path.data(), tag_class_to_extension(compiled_tag->tag_class_int));
                    network_issue = true;
                }
                #endif

                // Stub the tag
                compiled_tag->path = std::string("stub\\") + tag_class_to_extension(compiled_tag->tag_class_int) + "\\" + compiled_tag->path;
                compiled_tag->tag_class_int = TagClassInt::TAG_CLASS_UNICODE_STRING_LIST;
                compiled_tag->data.insert(compiled_tag->data.begin(), 12, std::byte());
            }
        }

        if(network_issue) {
            eprintf("Warning: The game will crash in multiplayer if missing tags are used.\n");
        }
    }

    std::size_t BuildWorkload::compile_tag_recursively(const char *path, HEK::TagClassInt tag_class_int) {
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
        std::size_t actual_path_size = std::snprintf(tag_base_path, MAX_PATH_LENGTH, "%s.%s", path, tag_class_to_extension(tag_class_int));
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
            // Open the tag file
            std::FILE *file = nullptr;

            // If it's not purely an object tag try to open it
            if(tag_class_int != TagClassInt::TAG_CLASS_OBJECT) {
                // Concatenate the tag path
                std::string tag_path = tag_dir + tag_base_path;
                file = std::fopen(tag_path.data(), "rb");
            }
            // Otherwise, see if we can go through the different object types
            else {
                auto attempt_to_open_tag = [&tag_dir, &path](TagClassInt class_int) -> std::FILE * {
                    std::string tag_path = tag_dir + path + "." + tag_class_to_extension(class_int);
                    #ifndef _WIN32
                    for(char &c : tag_path) {
                        if(c == '\\') {
                            c = '/';
                        }
                    }
                    #endif
                    return std::fopen(tag_path.data(), "rb");
                };
                #define MAKE_ATTEMPT(class_int) if(file == nullptr) { file = attempt_to_open_tag(class_int); if(file) tag_class_int = class_int; }

                std::string tag_path = tag_dir + tag_base_path;
                MAKE_ATTEMPT(HEK::TagClassInt::TAG_CLASS_BIPED);
                MAKE_ATTEMPT(HEK::TagClassInt::TAG_CLASS_DEVICE);
                MAKE_ATTEMPT(HEK::TagClassInt::TAG_CLASS_DEVICE_CONTROL);
                MAKE_ATTEMPT(HEK::TagClassInt::TAG_CLASS_DEVICE_LIGHT_FIXTURE);
                MAKE_ATTEMPT(HEK::TagClassInt::TAG_CLASS_DEVICE_MACHINE);
                MAKE_ATTEMPT(HEK::TagClassInt::TAG_CLASS_EQUIPMENT);
                MAKE_ATTEMPT(HEK::TagClassInt::TAG_CLASS_GARBAGE);
                MAKE_ATTEMPT(HEK::TagClassInt::TAG_CLASS_ITEM);
                MAKE_ATTEMPT(HEK::TagClassInt::TAG_CLASS_OBJECT);
                MAKE_ATTEMPT(HEK::TagClassInt::TAG_CLASS_PLACEHOLDER);
                MAKE_ATTEMPT(HEK::TagClassInt::TAG_CLASS_PROJECTILE);
                MAKE_ATTEMPT(HEK::TagClassInt::TAG_CLASS_SCENERY);
                MAKE_ATTEMPT(HEK::TagClassInt::TAG_CLASS_SOUND_SCENERY);
                MAKE_ATTEMPT(HEK::TagClassInt::TAG_CLASS_VEHICLE);
                MAKE_ATTEMPT(HEK::TagClassInt::TAG_CLASS_WEAPON);

                // If we successfully open it, try again
                if(file) {
                    std::fclose(file);
                    return compile_tag_recursively(path, tag_class_int);
                }
            }

            // Continue if we couldn't do it
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

            // Calculate pixel size for particles and weather particles
            auto get_bitmap_tag_pixel_size = [](std::unique_ptr<CompiledTag> &bitmap_tag) {
                auto &bitmap_tag_data = *reinterpret_cast<Bitmap<LittleEndian> *>(bitmap_tag->data.data());
                float pixel_size = 1.0F;

                // Get the dimensions of the bitmaps
                std::uint32_t bitmap_count = bitmap_tag_data.bitmap_data.count;
                std::vector<std::pair<std::uint16_t, std::uint16_t>> bitmap_dimensions(bitmap_count);
                auto *bitmap_data = reinterpret_cast<BitmapData<LittleEndian> *>(bitmap_tag->data.data() + bitmap_tag->resolve_pointer(&bitmap_tag_data.bitmap_data.pointer));
                for(std::uint32_t b = 0; b < bitmap_count; b++) {
                    bitmap_dimensions[b].first = bitmap_data[b].width;
                    bitmap_dimensions[b].second = bitmap_data[b].height;
                }

                // Get sequences
                auto sequence_offset = bitmap_tag->resolve_pointer(&bitmap_tag_data.bitmap_group_sequence.pointer);
                if(sequence_offset != INVALID_POINTER) {
                    for(std::size_t sequence_index = 0; sequence_index < bitmap_tag_data.bitmap_group_sequence.count; sequence_index++) {
                        auto &sequence = reinterpret_cast<BitmapGroupSequence<LittleEndian> *>(bitmap_tag->data.data() + sequence_offset)[sequence_index];
                        auto first_sprite_offset = bitmap_tag->resolve_pointer(&sequence.sprites.pointer);

                        // We'll need to iterate through all of the sprites
                        std::size_t sprite_count = sequence.sprites.count;
                        if(first_sprite_offset != INVALID_POINTER) {
                            for(std::size_t i = 0; i < sprite_count; i++) {
                                auto &sprite = reinterpret_cast<BitmapGroupSprite<LittleEndian> *>(bitmap_tag->data.data() + first_sprite_offset)[i];

                                // If the bitmap index is invalid, complain
                                if(static_cast<std::size_t>(sprite.bitmap_index) >= bitmap_count) {
                                    eprintf("Invalid bitmap index for sprite: %zu\n", static_cast<std::size_t>(bitmap_count));
                                    throw OutOfBoundsException();
                                }

                                // Get yer values here. Get 'em while they're hot.
                                float width_bitmap = 1.0F / std::fabs(sprite.right - sprite.left) / bitmap_dimensions[sprite.bitmap_index].first;
                                float height_bitmap = 1.0F / std::fabs(sprite.bottom - sprite.top) / bitmap_dimensions[sprite.bitmap_index].second;
                                float smallest = (width_bitmap < height_bitmap) ? width_bitmap : height_bitmap;
                                if(pixel_size > smallest) {
                                    pixel_size = smallest;
                                }
                            }
                        }
                    }
                }
                return pixel_size;
            };

            try {
                // Compile the tag
                std::unique_ptr<CompiledTag> tag(std::make_unique<CompiledTag>(path, tag_class_int, this->tag_buffer.data(), file_length, this->cache_file_type));
                CompiledTag *tag_ptr = tag.get();

                // If it's a scenario tag, set this
                if(tag->tag_class_int == HEK::TAG_CLASS_SCENARIO) {
                    this->cache_file_type = reinterpret_cast<Scenario<LittleEndian> *>(tag->data.data())->type;
                }

                // Insert into the tag array
                if(adding) {
                    this->compiled_tags.emplace_back(std::move(tag));
                }
                else {
                    this->compiled_tags[index] = std::move(tag);
                }

                // Iterate through all of the tags this tag references
                for(auto &dependency : tag_ptr->dependencies) {
                    // Older HEK tags may use .model references instead of .gbxmodel references. tool.exe changes them to .gbxmodel references.
                    if(dependency.tag_class_int == TagClassInt::TAG_CLASS_MODEL) {
                        dependency.tag_class_int = TagClassInt::TAG_CLASS_GBXMODEL;
                    }

                    // Compile the tag
                    TagID new_tag_id = tag_id_from_index(this->compile_tag_recursively(dependency.path.data(), dependency.tag_class_int));

                    // Set the tag ID
                    if(!dependency.tag_id_only) {
                        auto *dependency_in_tag = reinterpret_cast<TagDependency<LittleEndian> *>(tag_ptr->data.data() + dependency.offset);
                        dependency_in_tag->tag_id = new_tag_id;
                        dependency_in_tag->tag_class_int = dependency.tag_class_int;
                    }
                    else {
                        *reinterpret_cast<LittleEndian<TagID> *>(tag_ptr->data.data() + dependency.offset) = new_tag_id;
                    }
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
                                if(fog_id.is_null()) {
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

                // Detail-object collection things
                else if(tag_ptr->tag_class_int == HEK::TagClassInt::TAG_CLASS_DETAIL_OBJECT_COLLECTION) {
                    // Make sure we're referencing the right things
                    auto &dobc = *reinterpret_cast<DetailObjectCollection<LittleEndian> *>(tag_ptr->data.data());
                    if(dobc.sprite_plate.tag_id.read().is_null()) {
                        eprintf("%s.detail_object_collection has no bitmap.\n", tag_ptr->path.data());
                        throw;
                    }
                    auto &bitmap_tag = this->compiled_tags[dobc.sprite_plate.tag_id.read().index];
                    if(bitmap_tag->tag_class_int != TagClassInt::TAG_CLASS_BITMAP) {
                        eprintf("%s.detail_object_collection does not reference a bitmap.\n", tag_ptr->path.data());
                        throw;
                    }

                    // Get bitmap data
                    auto &bitmap = *reinterpret_cast<Bitmap<LittleEndian> *>(bitmap_tag->data.data());
                    std::uint32_t sequences_count = bitmap.bitmap_group_sequence.count.read();
                    auto *sequences = reinterpret_cast<BitmapGroupSequence<LittleEndian> *>(bitmap_tag->data.data() + bitmap_tag->resolve_pointer(&bitmap.bitmap_group_sequence.pointer));

                    // Go through each thing
                    std::uint32_t dobc_count = dobc.types.count.read();
                    auto *dobc_type = reinterpret_cast<DetailObjectCollectionObjectType<LittleEndian> *>(tag_ptr->data.data() + tag_ptr->resolve_pointer(&dobc.types.pointer));
                    for(std::uint32_t i = 0; i < dobc_count; i++) {
                        std::uint8_t sequence_index = dobc_type[i].sequence_index;
                        if(sequence_index >= sequences_count) {
                            eprintf("Invalid sequence index %u / %u.\n", sequence_index, sequences_count);
                            throw OutOfBoundsException();
                        }
                        dobc_type[i].sprite_count = static_cast<std::uint8_t>(sequences[sequence_index].sprites.count.read());
                    }
                }

                // Particle-related things
                else if(tag_ptr->tag_class_int == HEK::TagClassInt::TAG_CLASS_PARTICLE) {
                    auto &particle = *reinterpret_cast<Particle<LittleEndian> *>(tag_ptr->data.data());
                    if(particle.bitmap.tag_id.read().is_null()) {
                        eprintf("%s.particle has no bitmap.\n", tag_ptr->path.data());
                        throw;
                    }
                    else {
                        particle.unknown = get_bitmap_tag_pixel_size(this->compiled_tags[particle.bitmap.tag_id.read().index]);
                    }
                }

                // Weather-related things
                else if(tag_ptr->tag_class_int == HEK::TagClassInt::TAG_CLASS_WEATHER_PARTICLE_SYSTEM) {
                    auto &weather_particle_system = *reinterpret_cast<WeatherParticleSystem<LittleEndian> *>(tag_ptr->data.data());
                    std::uint32_t particle_count = weather_particle_system.particle_types.count;
                    if(particle_count > 0) {
                        std::size_t offset = tag_ptr->resolve_pointer(&weather_particle_system.particle_types.pointer);
                        auto *types = reinterpret_cast<WeatherParticleSystemParticleType<LittleEndian> *>(tag_ptr->data.data() + offset);
                        for(std::uint32_t p = 0; p < particle_count; p++) {
                            auto &type = types[p];
                            if(type.sprite_bitmap.tag_id.read().is_null()) {
                                eprintf("%s.weather_particle_system particle #%zu has no bitmap.\n", tag_ptr->path.data(), static_cast<std::size_t>(p));
                                throw;
                            }
                            else {
                                type.unknown = get_bitmap_tag_pixel_size(this->compiled_tags[type.sprite_bitmap.tag_id.read().index]);
                            }
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
                                bsp_data->runtime_decals.count = static_cast<std::uint32_t>(runtime_decals.size());

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

                    // If it's a biped, set this value
                    if(tag_ptr->tag_class_int == HEK::TAG_CLASS_BIPED) {
                        auto &biped = *reinterpret_cast<HEK::Biped<HEK::LittleEndian> *>(tag_ptr->data.data());
                        auto biped_model = biped.model.tag_id.read();
                        if(biped_model.is_null()) {
                            // If no model, set to 0xFFFF
                            biped.head_model_node_index = static_cast<std::uint16_t>(0xFFFF);
                        }
                        else {
                            auto &model_tag = this->compiled_tags[biped_model.index];
                            if(model_tag->tag_class_int == HEK::TAG_CLASS_GBXMODEL) {
                                auto &model = *reinterpret_cast<HEK::GBXModel<HEK::LittleEndian> *>(model_tag->data.data());
                                auto *nodes = reinterpret_cast<HEK::GBXModelNode<HEK::LittleEndian> *>(model_tag->data.data() + model_tag->resolve_pointer(&model.nodes.pointer));
                                std::size_t node_count = model.nodes.count.read();
                                for(std::size_t node = 0; node < node_count; node++) {
                                    if(nodes[node].name.string[31] == 0 && std::strcmp(nodes[node].name.string, "bip01 head") == 0) {
                                        biped.head_model_node_index = static_cast<std::uint16_t>(node);
                                        break;
                                    }
                                }
                            }

                            // If for some reason this isn't a model tag we can recognize, set to 0xFFFF
                            else {
                                biped.head_model_node_index = static_cast<std::uint16_t>(0xFFFF);
                            }
                        }
                    }
                }

                return index;
            }
            catch(...) {
                eprintf("Failed to compile %s.%s\n", path, tag_class_to_extension(tag_class_int));
                throw;
            }
        }

        eprintf("Could not find %s.%s\n", path, tag_class_to_extension(tag_class_int));
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
            eprintf("Scenario name %s exceeds 31 characters.\n", map_name.data());
            throw InvalidScenarioNameException();
        }

        // Lowercase everything
        for(char &c : map_name) {
            c = std::tolower(c);
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
                        eprintf("Invalid BSP reference in scenario tag.\n");
                        throw InvalidDependencyException();
                    }

                    // Check if it's a BSP tag
                    auto &bsp_compiled_tag = this->compiled_tags[bsp_index];
                    if(bsp_compiled_tag->tag_class_int != TagClassInt::TAG_CLASS_SCENARIO_STRUCTURE_BSP) {
                        eprintf("Mismatched BSP reference in scenario tag.\n");
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
                eprintf("Invalid pointer for %s.%s\n", compiled_tag->path.data(), tag_class_to_extension(compiled_tag->tag_class_int));
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
            // Skip tag ID-only dependencies
            if(dependency.tag_id_only) {
                continue;
            }

            if(dependency.offset + sizeof(TagDependency<LittleEndian>) > compiled_tag->data.size()) {
                eprintf("Invalid dependency offset for %s.%s\n", compiled_tag->path.data(), tag_class_to_extension(compiled_tag->tag_class_int));
                throw InvalidDependencyException();
            }
            auto &dependency_data = *reinterpret_cast<TagDependency<LittleEndian> *>(tag_data_data + dependency.offset);

            // Resolve the dependency
            std::size_t depended_tag_id = dependency_data.tag_id.read().index;
            if(depended_tag_id >= this->tag_count) {
                eprintf("Invalid dependency index for %s.%s\n", compiled_tag->path.data(), tag_class_to_extension(compiled_tag->tag_class_int));
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
                            eprintf("Invalid pixels offset for bitmap %zu for %s.%s\n", b, this->compiled_tags[i]->path.data(), tag_class_to_extension(this->compiled_tags[i]->tag_class_int));
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
                        eprintf("Invalid vertex size for part %zu-%zu for %s.%s\n", g, p, this->compiled_tags[i]->path.data(), tag_class_to_extension(this->compiled_tags[i]->tag_class_int));
                        throw OutOfBoundsException();
                    }

                    std::size_t index_size = (part.triangle_count + 2) * sizeof(std::uint16_t);
                    if(index_size + index_offset > model_data_size) {
                        eprintf("Invalid index size for part %zu-%zu for %s.%s\n", g, p, this->compiled_tags[i]->path.data(), tag_class_to_extension(this->compiled_tags[i]->tag_class_int));
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

    #define TRANSLATE_SCENARIO_TAG_DATA_PTR(pointer) (scenario_tag_data + scenario_tag->resolve_pointer(&pointer))
    #define TRANSLATE_SBSP_TAG_DATA_PTR(pointer) (sbsp_tag_data + sbsp_tag->resolve_pointer(&pointer))

    void BuildWorkload::fix_scenario_tag_encounters() {
        // Get the scenario tag and some BSP information
        auto &scenario_tag = this->compiled_tags[this->scenario_index];
        auto *scenario_tag_data = scenario_tag->data.data();
        auto &scenario = *reinterpret_cast<HEK::Scenario<HEK::LittleEndian> *>(scenario_tag_data);
        std::uint32_t sbsp_count = scenario.structure_bsps.count;

        // Get encounters
        std::uint32_t encounters_count = scenario.encounters.count;
        auto *encounters = reinterpret_cast<HEK::ScenarioEncounter<HEK::LittleEndian> *>(TRANSLATE_SCENARIO_TAG_DATA_PTR(scenario.encounters.pointer));
        auto *encounters_end = encounters + encounters_count;

        // Set some stuff up
        static constexpr std::size_t VECTOR_SIZE = 65536;
        auto clear_array = [](std::unique_ptr<std::uint8_t []> &arr) {
            std::fill(arr.get(), arr.get() + VECTOR_SIZE, 0);
        };
        auto copy_array = [](const std::unique_ptr<std::uint8_t []> &from, std::unique_ptr<std::uint8_t []> &to) {
            std::copy(from.get(), from.get() + VECTOR_SIZE, to.get());
        };
        auto set_flag_for_array = [](std::unique_ptr<std::uint8_t []> &arr, std::size_t offset, std::uint8_t flag) {
            if(offset < VECTOR_SIZE) {
                arr[offset] = flag;
            }
        };

        // This is an array that holds 0's and 1's depending on a hit or miss. Best means the best for a given encounter. Current means for a given BSP.
        // Ideally you want everything to be 1's up to the total count. 0 means it wasn't found in a BSP.
        auto best_firing_position = std::make_unique<std::uint8_t []>(VECTOR_SIZE);
        auto best_squad = std::make_unique<std::uint8_t []>(VECTOR_SIZE);
        auto current_firing_position = std::make_unique<std::uint8_t []>(VECTOR_SIZE);
        auto current_squad = std::make_unique<std::uint8_t []>(VECTOR_SIZE);

        // Iterate
        std::size_t warnings_given = 0;
        for(auto *encounter = encounters; encounter < encounters_end; encounter++) {
            // Clear the best arrays
            clear_array(best_firing_position);
            clear_array(best_squad);

            // If the BSP index was manually specified, use that.
            if(encounter->flags.read().manual_bsp_index_specified) {
                encounter->precomputed_bsp_index = encounter->manual_bsp_index;
                continue;
            }

            // Highest number of hits in a BSP and current number of hits
            std::uint32_t highest_count = 0;

            // How many BSPs we've found the highest count
            std::uint32_t bsps_found_in = 0;

            // Get pointers
            auto *squads = reinterpret_cast<HEK::ScenarioSquad<HEK::LittleEndian> *>(TRANSLATE_SCENARIO_TAG_DATA_PTR(encounter->squads.pointer));
            auto *firing_positions = reinterpret_cast<HEK::ScenarioFiringPosition<HEK::LittleEndian> *>(TRANSLATE_SCENARIO_TAG_DATA_PTR(encounter->firing_positions.pointer));

            std::uint32_t squad_max_hits = 0;
            std::uint32_t squad_total = encounter->squads.count.read();
            std::uint32_t firing_position_max_hits = 0;
            std::uint32_t firing_position_total = encounter->firing_positions.count.read();
            std::uint32_t max_hits = squad_total + firing_position_total;

            float distance_to_ground_max = 0.0F;

            // Hold onto this
            bool three_dimensional_fire_positions = encounter->flags.read()._3d_firing_positions;

            // Begin counting and iterating through the BSPs
            for(std::uint32_t b = 0; b < sbsp_count; b++) {
                // Clear the current arrays
                clear_array(current_firing_position);
                clear_array(current_squad);

                // Don't check for stuff here if there's nothing to check
                if(max_hits == 0) {
                    break;
                }

                // Check if the squad spawn positions are in the BSP
                std::uint32_t squad_count = 0;

                const float MAX_DISTANCE = 2.0F;
                float distance_to_ground = MAX_DISTANCE;
                for(std::size_t s = 0; s < squad_total; s++) {
                    // Add 'em up
                    auto &squad = squads[s];

                    // Have this here
                    bool found = true;

                    // Check spawn points
                    auto *spawn_point_first = reinterpret_cast<HEK::ScenarioActorStartingLocation<HEK::LittleEndian> *>(TRANSLATE_SCENARIO_TAG_DATA_PTR(squad.starting_locations.pointer));
                    auto *spawn_point_end = spawn_point_first + squad.starting_locations.count.read();
                    for(auto *spawn_point = spawn_point_first; spawn_point < spawn_point_end && found; spawn_point++) {
                        if(!this->point_in_bsp(b, spawn_point->position)) {
                            found = false;
                        }

                        // If 3D firing positions is not set, find the distance to the ground.
                        else if(!three_dimensional_fire_positions) {
                            // Raycasting; Get two points: one at the point and one MAX_DISTANCE below it
                            HEK::Point3D<HEK::LittleEndian> point = spawn_point->position;
                            HEK::Point3D<HEK::LittleEndian> point_lower = spawn_point->position;
                            point_lower.z = point.z - MAX_DISTANCE;

                            // Use raycasting to get the distance to ground
                            float q = MAX_DISTANCE;
                            HEK::Point3D<HEK::LittleEndian> point_intersect;
                            std::uint32_t surface_index, leaf_index;
                            if(this->intersect_in_bsp(point, point_lower, b, point_intersect, surface_index, leaf_index)) {
                                q = point.z - point_intersect.z;
                            }
                            else {
                                found = false;
                                break;
                            }

                            // If what we got is less than what we had before, narrow it down like that
                            if(distance_to_ground > q) {
                                distance_to_ground = q;
                            }
                        }
                    }

                    if(found) {
                        squad_count++;
                        set_flag_for_array(current_squad, s, 1);
                    }
                }

                // Check if the firing positions are in the BSP
                std::uint32_t firing_position_count = 0;
                for(std::size_t i = 0; i < firing_position_total; i++) {
                    if(this->point_in_bsp(b, firing_positions[i].position)) {
                        HEK::Point3D<HEK::LittleEndian> point = firing_positions[i].position;
                        HEK::Point3D<HEK::LittleEndian> point_lower = firing_positions[i].position;
                        point_lower.z = point.z - MAX_DISTANCE;

                        HEK::Point3D<HEK::LittleEndian> point_intersect;
                        std::uint32_t surface_index, leaf_index;

                        if(three_dimensional_fire_positions || this->intersect_in_bsp(point, point_lower, b, point_intersect, surface_index, leaf_index)) {
                            firing_position_count++;
                            set_flag_for_array(current_firing_position, i, 1);
                        }
                    }
                }

                // Check if we got something
                std::uint32_t current_count = squad_count + firing_position_count;
                if(current_count) {
                    if(current_count == highest_count) {
                        if(distance_to_ground == distance_to_ground_max) {
                            bsps_found_in++;
                        }
                        else if(distance_to_ground < distance_to_ground_max) {
                            bsps_found_in = 1;
                            encounter->precomputed_bsp_index = static_cast<std::uint16_t>(b);
                        }
                    }
                    else if(current_count > highest_count) {
                        encounter->precomputed_bsp_index = static_cast<std::uint16_t>(b);
                        bsps_found_in = 1;
                        highest_count = current_count;
                        squad_max_hits = squad_count;
                        firing_position_max_hits = firing_position_count;
                        distance_to_ground_max = distance_to_ground;
                        copy_array(current_firing_position, best_firing_position);
                        copy_array(current_squad, best_squad);
                    }
                }
            }

            if(max_hits == 0) {
                encounter->precomputed_bsp_index = static_cast<std::uint16_t>(0xFFFF);
            }
            else if(bsps_found_in > 1) {
                eprintf("Warning: Encounter #%zu (%s) was found in %u BSPs (will place in BSP #%u).\n", static_cast<std::size_t>(encounter - encounters), encounter->name.string, bsps_found_in, encounter->precomputed_bsp_index.read());
                warnings_given++;
            }
            else if(bsps_found_in == 0) {
                eprintf("Warning: Encounter #%zu (%s) was found in 0 BSPs.\n", static_cast<std::size_t>(encounter - encounters), encounter->name.string);
                warnings_given++;
                encounter->precomputed_bsp_index = static_cast<std::uint16_t>(0xFFFF);
            }

            // Go through all the firing positions and set cluster stuff
            std::uint16_t bsp = encounter->precomputed_bsp_index.read();
            if(bsp != 0xFFFF && firing_position_total > 0) {
                // Get leaves for the BSP
                auto &sbsp_tag = this->compiled_tags[this->get_bsp_tag_index(bsp)];
                auto *sbsp_tag_data = sbsp_tag->data.data();
                auto &sbsp_tag_header = *reinterpret_cast<HEK::ScenarioStructureBSPCompiledHeader *>(sbsp_tag_data);
                auto &sbsp = *reinterpret_cast<HEK::ScenarioStructureBSP<HEK::LittleEndian> *>(TRANSLATE_SBSP_TAG_DATA_PTR(sbsp_tag_header.pointer));

                // Make sure we have a collision BSP
                if(sbsp.collision_bsp.count != 1) {
                    eprintf("BSP is missing a collision BSP.\n");
                    throw OutOfBoundsException();
                }
                auto &collision_bsp = *reinterpret_cast<HEK::ModelCollisionGeometryBSP<HEK::LittleEndian> *>(TRANSLATE_SBSP_TAG_DATA_PTR(sbsp.collision_bsp.pointer));

                // Leaves
                auto *render_leaves = reinterpret_cast<HEK::ScenarioStructureBSPLeaf<HEK::LittleEndian> *>(TRANSLATE_SBSP_TAG_DATA_PTR(sbsp.leaves.pointer));
                auto leaves_count = sbsp.leaves.count.read();
                if(collision_bsp.leaves.count != leaves_count) {
                    eprintf("BSP leaf count is not correct.\n");
                    throw OutOfBoundsException();
                }

                for(std::uint32_t f = 0; f < firing_position_total; f++) {
                    // Get a reference to the firing position
                    auto &firing_position = firing_positions[f];

                    // Get the leaf this point is located in
                    auto leaf = this->leaf_for_point_in_bsp(bsp, firing_position.position);
                    if(leaf.is_null()) {
                        firing_position.cluster_index = 0xFFFF;
                        continue;
                    }

                    // If it's 3D firing positions, then we can't get the surface index
                    else if(three_dimensional_fire_positions) {
                        firing_position.cluster_index = render_leaves[leaf.int_value()].cluster;
                        firing_position.surface_index = ~0;
                        continue;
                    }

                    if(leaves_count <= leaf) {
                        eprintf("Invalid leaf index %u / %u in BSP.\n", leaf.int_value(), leaves_count);
                        throw OutOfBoundsException();
                    }

                    // Intersect
                    auto position_below = firing_position.position;
                    position_below.z = position_below.z - 0.5F;

                    HEK::Point3D<HEK::LittleEndian> intersection_point;
                    std::uint32_t surface_index, leaf_index;
                    if(intersect_in_bsp(firing_position.position, position_below, bsp, intersection_point, surface_index, leaf_index)) {
                        firing_position.cluster_index = render_leaves[leaf_index].cluster;
                        firing_position.surface_index = surface_index;
                    }
                }
            }

            if(max_hits > highest_count) {
                eprintf("Warning: Encounter #%zu (%s) is partially outside of BSP #%u (%u / %u hits).\n", static_cast<std::size_t>(encounter - encounters), encounter->name.string, encounter->precomputed_bsp_index.read(), highest_count, max_hits);
                warnings_given++;

                // Show some information
                auto print_info_for_encounter = [](const char *name, std::uint32_t max, std::uint32_t total, const std::unique_ptr<std::uint8_t []> &best) {
                    char indices_list[256] = {};
                    char *indices_list_offset = indices_list;
                    char *indices_list_end = indices_list + sizeof(indices_list) - 1;
                    int counted = 0;
                    for(std::size_t i = 0; i < total && i < VECTOR_SIZE; i++) {
                        if(!best.get()[i]) {
                            if(counted == 5) {
                                std::snprintf(indices_list_offset, indices_list_end - indices_list_offset, ", ...");
                                break;
                            }
                            else {
                                const char *comma = ", ";
                                if(counted == 0) {
                                    comma = "";
                                }
                                indices_list_offset += std::snprintf(indices_list_offset, indices_list_end - indices_list_offset, "%s%zu", comma, i);
                                counted++;
                            }
                        }
                    }
                    char counter[16] = {};
                    std::snprintf(counter, sizeof(counter), "%u/%u", max, total);
                    eprintf("    %-20s Count: %-16s Indices: [%s]\n", name, counter, indices_list);
                };

                print_info_for_encounter("squads", squad_max_hits, squad_total, best_squad);
                print_info_for_encounter("firing positions", firing_position_max_hits, firing_position_total, best_firing_position);
            }
        }

        if(warnings_given) {
            eprintf("Note: You can use manual BSP indices to silence %s.\n", warnings_given == 1 ? "this warning" : "these warnings");
        }
    }

    void BuildWorkload::fix_scenario_tag_command_lists() {
        // Get the scenario tag and some BSP information
        auto &scenario_tag = this->compiled_tags[this->scenario_index];
        auto *scenario_tag_data = scenario_tag->data.data();
        auto &scenario = *reinterpret_cast<HEK::Scenario<HEK::LittleEndian> *>(scenario_tag_data);
        std::uint32_t sbsp_count = scenario.structure_bsps.count;

        // Get command lists
        std::uint32_t command_lists_count = scenario.command_lists.count;
        auto *command_lists = reinterpret_cast<HEK::ScenarioCommandList<HEK::LittleEndian> *>(TRANSLATE_SCENARIO_TAG_DATA_PTR(scenario.command_lists.pointer));

        std::size_t warnings_given = 0;

        // Iterate
        for(std::uint32_t c = 0; c < command_lists_count; c++) {
            auto &command_list = command_lists[c];

            // If a manual BSP index is set or we risk dividing by 0, skip it
            if(command_list.flags.read().manual_bsp_index) {
                command_list.precomputed_bsp_index = command_list.manual_bsp_index;
                continue;
            }
            if(command_list.points.count == 0) {
                command_list.precomputed_bsp_index = ~static_cast<std::uint16_t>(0);
                continue;
            }

            // Otherwise, let's begin
            std::uint32_t points_count = command_list.points.count;
            auto *points = reinterpret_cast<HEK::ScenarioCommandPoint<HEK::LittleEndian> *>(TRANSLATE_SCENARIO_TAG_DATA_PTR(command_list.points.pointer));
            std::uint32_t points_found = 0;
            std::uint32_t highest_bsp = 0;
            std::uint32_t highest_bsp_count = 0;

            for(std::uint32_t b = 0; b < sbsp_count; b++) {
                std::uint32_t point_count_this_bsp = 0;
                for(std::uint32_t p = 0; p < points_count; p++) {
                    point_count_this_bsp += this->point_in_bsp(b, points[p].position);
                }
                if(point_count_this_bsp > 0) {
                    if(points_found == point_count_this_bsp) {
                        highest_bsp_count++;
                    }
                    else if(point_count_this_bsp > points_found) {
                        highest_bsp = b;
                        highest_bsp_count = 1;
                        points_found = point_count_this_bsp;
                    }
                }
            }

            command_list.precomputed_bsp_index = highest_bsp;

            if(highest_bsp_count == 0) {
                eprintf("Warning: Command list #%u (%s) was found in 0 BSPs.\n", c, command_list.name.string);
                warnings_given++;
            }
            else {
                if(points_found < points_count) {
                    eprintf("Warning: Command list #%u (%s) is partially outside of BSP #%u (%u / %u hits).\n", c, command_list.name.string, highest_bsp, points_found, points_count);
                    warnings_given++;
                }
                else if(highest_bsp_count > 1) {
                    eprintf("Warning: Command list #%u (%s) was found in %u BSPs (will place in BSP #%u).\n", c, command_list.name.string, highest_bsp_count, highest_bsp);
                    warnings_given++;
                }
            }
        }

        if(warnings_given) {
            eprintf("Note: You can use manual BSP indices to silence %s.\n", warnings_given == 1 ? "this warning" : "these warnings");
        }
    }

    HEK::FlaggedInt<std::uint32_t> BuildWorkload::leaf_for_point_in_bsp(std::uint32_t bsp, const HEK::Point3D<HEK::LittleEndian> &point) {
        // Get current BSP data
        auto &sbsp_tag = this->compiled_tags[this->get_bsp_tag_index(bsp)];
        auto *sbsp_tag_data = sbsp_tag->data.data();
        auto &sbsp_tag_header = *reinterpret_cast<HEK::ScenarioStructureBSPCompiledHeader *>(sbsp_tag_data);
        auto &sbsp = *reinterpret_cast<HEK::ScenarioStructureBSP<HEK::LittleEndian> *>(TRANSLATE_SBSP_TAG_DATA_PTR(sbsp_tag_header.pointer));

        // Make sure we have a collision BSP
        if(sbsp.collision_bsp.count != 1) {
            eprintf("BSP is missing a collision BSP.\n");
            throw OutOfBoundsException();
        }
        auto &collision_bsp = *reinterpret_cast<HEK::ModelCollisionGeometryBSP<HEK::LittleEndian> *>(TRANSLATE_SBSP_TAG_DATA_PTR(sbsp.collision_bsp.pointer));

        // Get BSP3D nodes and planes
        auto *bsp3d_nodes = reinterpret_cast<HEK::ModelCollisionGeometryBSP3DNode<HEK::LittleEndian> *>(TRANSLATE_SBSP_TAG_DATA_PTR(collision_bsp.bsp3d_nodes.pointer));
        auto bsp3d_nodes_count = collision_bsp.bsp3d_nodes.count.read();
        auto *planes = reinterpret_cast<HEK::ModelCollisionGeometryPlane<HEK::LittleEndian> *>(TRANSLATE_SBSP_TAG_DATA_PTR(collision_bsp.planes.pointer));
        auto planes_count = collision_bsp.planes.count.read();

        return HEK::leaf_for_point_of_bsp_tree(point, bsp3d_nodes, bsp3d_nodes_count, planes, planes_count);
    }

    bool BuildWorkload::intersect_in_bsp(const HEK::Point3D<HEK::LittleEndian> &point_a, const HEK::Point3D<HEK::LittleEndian> &point_b, std::uint32_t bsp, HEK::Point3D<HEK::LittleEndian> &intersection_point, std::uint32_t &surface_index, std::uint32_t &leaf_index) {
        auto &sbsp_tag = this->compiled_tags[this->get_bsp_tag_index(bsp)];
        auto *sbsp_tag_data = sbsp_tag->data.data();
        auto &sbsp_tag_header = *reinterpret_cast<HEK::ScenarioStructureBSPCompiledHeader *>(sbsp_tag_data);
        auto &sbsp = *reinterpret_cast<HEK::ScenarioStructureBSP<HEK::LittleEndian> *>(TRANSLATE_SBSP_TAG_DATA_PTR(sbsp_tag_header.pointer));

        auto &collision_bsp = *reinterpret_cast<HEK::ModelCollisionGeometryBSP<HEK::LittleEndian> *>(TRANSLATE_SBSP_TAG_DATA_PTR(sbsp.collision_bsp.pointer));
        auto *bsp3d_nodes = reinterpret_cast<HEK::ModelCollisionGeometryBSP3DNode<HEK::LittleEndian> *>(TRANSLATE_SBSP_TAG_DATA_PTR(collision_bsp.bsp3d_nodes.pointer));
        auto bsp3d_nodes_count = collision_bsp.bsp3d_nodes.count.read();
        auto *planes = reinterpret_cast<HEK::ModelCollisionGeometryPlane<HEK::LittleEndian> *>(TRANSLATE_SBSP_TAG_DATA_PTR(collision_bsp.planes.pointer));
        auto planes_count = collision_bsp.planes.count.read();
        auto *leaves = reinterpret_cast<HEK::ModelCollisionGeometryLeaf<HEK::LittleEndian> *>(TRANSLATE_SBSP_TAG_DATA_PTR(collision_bsp.leaves.pointer));
        auto leaf_count = collision_bsp.leaves.count.read();
        auto *bsp2d_nodes = reinterpret_cast<HEK::ModelCollisionGeometryBSP2DNode<HEK::LittleEndian> *>(TRANSLATE_SBSP_TAG_DATA_PTR(collision_bsp.bsp2d_nodes.pointer));
        auto bsp2d_node_count = collision_bsp.bsp2d_nodes.count.read();
        auto *bsp2d_references = reinterpret_cast<HEK::ModelCollisionGeometryBSP2DReference<HEK::LittleEndian> *>(TRANSLATE_SBSP_TAG_DATA_PTR(collision_bsp.bsp2d_references.pointer));
        auto bsp2d_reference_count = collision_bsp.bsp2d_references.count.read();
        auto *surfaces = reinterpret_cast<HEK::ModelCollisionGeometrySurface<HEK::LittleEndian> *>(TRANSLATE_SBSP_TAG_DATA_PTR(collision_bsp.surfaces.pointer));
        auto surface_count = collision_bsp.surfaces.count.read();
        auto *edges = reinterpret_cast<HEK::ModelCollisionGeometryEdge<HEK::LittleEndian> *>(TRANSLATE_SBSP_TAG_DATA_PTR(collision_bsp.edges.pointer));
        auto edge_count = collision_bsp.edges.count.read();
        auto *vertices = reinterpret_cast<HEK::ModelCollisionGeometryVertex<HEK::LittleEndian> *>(TRANSLATE_SBSP_TAG_DATA_PTR(collision_bsp.vertices.pointer));
        auto vertex_count = collision_bsp.vertices.count.read();

        return check_for_intersection(point_a, point_b, bsp3d_nodes, bsp3d_nodes_count, planes, planes_count, leaves, leaf_count, bsp2d_nodes, bsp2d_node_count, bsp2d_references, bsp2d_reference_count, surfaces, surface_count, edges, edge_count, vertices, vertex_count, intersection_point, surface_index, leaf_index);
    }

    bool BuildWorkload::point_in_bsp(std::uint32_t bsp, const HEK::Point3D<HEK::LittleEndian> &point) {
        return !this->leaf_for_point_in_bsp(bsp, point).is_null();
    }

    std::size_t BuildWorkload::get_bsp_tag_index(std::uint32_t bsp) {
        auto &scenario_tag = this->compiled_tags[this->scenario_index];
        auto *scenario_tag_data = scenario_tag->data.data();
        auto &scenario = *reinterpret_cast<HEK::Scenario<HEK::LittleEndian> *>(scenario_tag_data);
        auto *sbsps = reinterpret_cast<HEK::ScenarioBSP<HEK::LittleEndian> *>(TRANSLATE_SCENARIO_TAG_DATA_PTR(scenario.structure_bsps.pointer));

        // Invalid BSP?
        if(scenario.structure_bsps.count <= bsp) {
            throw OutOfBoundsException();
        }

        // Do it
        auto &sbsp_scenario_entry = sbsps[bsp];
        auto sbsp_tag = sbsp_scenario_entry.structure_bsp.tag_id.read().index;
        if(sbsp_tag >= this->tag_count) {
            throw OutOfBoundsException();
        }

        return sbsp_tag;
    }
}
