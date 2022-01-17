// SPDX-License-Identifier: GPL-3.0-only

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <invader/version.hpp>
#include <invader/build/build_workload.hpp>
#include <invader/tag/hek/definition.hpp>
#include <invader/resource/resource_map.hpp>
#include <invader/resource/hek/resource_map.hpp>
#include <invader/resource/list/resource_list.hpp>
#include <invader/command_line_option.hpp>
#include <invader/file/file.hpp>
#include <invader/printf.hpp>

int main(int argc, const char **argv) {
    using namespace Invader::HEK;
    using namespace Invader;

    std::vector<CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("type", 'T', 1, "Set the resource map. This option is required. Can be: bitmaps, sounds, or loc.", "<type>");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory. Use multiple times to add more directories, ordered by precedence.", "<dir>");
    options.emplace_back("maps", 'm', 1, "Set the maps directory.", "<dir>");
    options.emplace_back("game-engine", 'g', 1, "Specify the game engine. This option is required. Demo and retail maps also require either --with-index or --with-map to be specified at least once. Valid engines are: gbx-custom, gbx-demo, gbx-retail, mcc-cea.", "<id>");
    options.emplace_back("with-index", 'w', 1, "Use an index file for the tags, ensuring tags are ordered in the same way (barring duplicates).", "<file>");
    options.emplace_back("with-map", 'M', 1, "Use a map file for the tags. This can be specified multiple times.", "<file>");
    options.emplace_back("concatenate", 'c', 1, "Concatenate against the resource map at a path. This cannot be used with -T loc", "<file>");
    options.emplace_back("show-matched", 'S', 0, "Print the paths of any matched tags found when using --concatenate.");

    static constexpr char DESCRIPTION[] = "Create resource maps.";
    static constexpr char USAGE[] = "[options] -T <type>";

    struct ResourceOption {
        // Tags directory
        std::vector<std::filesystem::path> tags;

        // Maps directory
        const char *maps = "maps";

        // Resource map type
        std::optional<ResourceMapType> type;

        // Engine target
        std::optional<HEK::CacheFileEngine> engine_target;

        const char * const *(*default_fn)() = get_default_bitmap_resources;
        std::vector<std::pair<const char *, bool>> index; // path, and is it a map?

        std::optional<std::filesystem::path> concatenate_against;

        // Spam console?
        bool show_matched = false;
    } resource_options;

    auto remaining_arguments = CommandLineOption::parse_arguments<ResourceOption &>(argc, argv, options, USAGE, DESCRIPTION, 0, 0, resource_options, [](char opt, const std::vector<const char *> &arguments, auto &resource_options) {
        switch(opt) {
            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);

            case 't':
                resource_options.tags.push_back(arguments[0]);
                break;

            case 'c':
                resource_options.concatenate_against = arguments[0];
                break;

            case 'S':
                resource_options.show_matched = true;
                break;

            case 'm':
                resource_options.maps = arguments[0];
                break;

            case 'g':
                if(std::strcmp(arguments[0], "gbx-custom") == 0) {
                    resource_options.engine_target = HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION;
                }
                else if(std::strcmp(arguments[0], "gbx-retail") == 0 || std::strcmp(arguments[0], "gbx-demo") == 0 || std::strcmp(arguments[0], "mcc-cea") == 0) {
                    resource_options.engine_target = HEK::CacheFileEngine::CACHE_FILE_RETAIL;
                }
                else {
                    eprintf_error("Invalid engine %s. Use --help for more information.", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;

            case 'w':
                resource_options.index.emplace_back(arguments[0], false);
                break;

            case 'M':
                resource_options.index.emplace_back(arguments[0], true);
                break;

            case 'T':
                if(std::strcmp(arguments[0], "bitmaps") == 0) {
                    resource_options.type = ResourceMapType::RESOURCE_MAP_BITMAP;
                    resource_options.default_fn = get_default_bitmap_resources;
                }
                else if(std::strcmp(arguments[0], "sounds") == 0) {
                    resource_options.type = ResourceMapType::RESOURCE_MAP_SOUND;
                    resource_options.default_fn = get_default_sound_resources;
                }
                else if(std::strcmp(arguments[0], "loc") == 0) {
                    resource_options.type = ResourceMapType::RESOURCE_MAP_LOC;
                    resource_options.default_fn = get_default_loc_resources;
                }
                else {
                    eprintf_error("Invalid type %s. Use --help for more information.", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;
        }
    });

    if(!resource_options.type.has_value()) {
        eprintf_error("No resource map type was given. Use -h for more information.");
        return EXIT_FAILURE;
    }

    if(!resource_options.engine_target.has_value()) {
        eprintf_error("No game engine was set. Use -h for more information.");
        return EXIT_FAILURE;
    }

    bool retail = *resource_options.engine_target == HEK::CacheFileEngine::CACHE_FILE_RETAIL;
    if(retail && resource_options.type == ResourceMapType::RESOURCE_MAP_LOC) {
        eprintf_error("Only bitmaps.map and sounds.map can be made for gbx-demo, gbx-retail, or mcc-cea engines.");
        return EXIT_FAILURE;
    }

    if(retail && resource_options.index.size() == 0) {
        eprintf_error("A map or index is required for building gbx-demo, gbx-retail, or mcc-cea maps.");
        return EXIT_FAILURE;
    }

    // If we don't have any tags directories, use a default one
    if(resource_options.tags.size() == 0) {
        resource_options.tags.push_back("tags");
    }

    // Generate our list
    std::vector<File::TagFilePath> tags_list;

    // First, add all default indices if we're Custom Edition
    if(resource_options.engine_target == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION) {
        for(const char * const *i = resource_options.default_fn(); *i; i++) {
            tags_list.emplace_back(File::split_tag_class_extension(*i).value());
        }
    }

    // Next, get all the tags
    if(resource_options.index.size()) {
        for(auto &index : resource_options.index) {
            if(index.second) {
                // Open the map first
                auto map_file = File::open_file(index.first);
                if(map_file.has_value()) {
                    auto map = Map::map_with_move(std::move(*map_file));
                    auto tag_count = map.get_tag_count();
                    for(std::size_t t = 0; t < tag_count; t++) {
                        auto &tag = map.get_tag(t);
                        bool allowed = false;
                        auto tag_class = tag.get_tag_fourcc();
                        switch(*resource_options.type) {
                            case ResourceMapType::RESOURCE_MAP_BITMAP:
                                allowed = tag_class == HEK::TagFourCC::TAG_FOURCC_BITMAP;
                                break;
                            case ResourceMapType::RESOURCE_MAP_SOUND:
                                allowed = tag_class == HEK::TagFourCC::TAG_FOURCC_SOUND;
                                break;
                            case ResourceMapType::RESOURCE_MAP_LOC:
                                allowed = tag_class == HEK::TagFourCC::TAG_FOURCC_HUD_MESSAGE_TEXT || tag_class == HEK::TagFourCC::TAG_FOURCC_UNICODE_STRING_LIST || tag_class == HEK::TagFourCC::TAG_FOURCC_FONT;
                                break;
                        }
                        if(allowed) {
                            tags_list.emplace_back(tag.get_path(), tag_class);
                        }
                    }
                }
                else {
                    eprintf_error("Failed to open %s\n", index.first);
                    return EXIT_FAILURE;
                }
            }
            else {
                // Add an index
                std::fstream index_file(index.first, std::ios_base::in);
                std::string tag;
                while(std::getline(index_file, tag)) {
                    auto split_path_maybe = File::split_tag_class_extension(tag);
                    if(split_path_maybe.has_value()) {
                        tags_list.emplace_back(*split_path_maybe);
                    }
                    else {
                        auto &new_path = tags_list.emplace_back();
                        new_path.path = tag;
                        new_path.fourcc = TagFourCC::TAG_FOURCC_NONE;
                    }
                }
            }
        }
    }

    ResourceMapHeader header = {};
    header.type = *resource_options.type;
    header.resource_count = tags_list.size();

    // Read the amazing fun happy stuff
    std::vector<std::byte> resource_data(sizeof(ResourceMapHeader));

    std::vector<Resource> concatenate_resource;
    if(resource_options.concatenate_against.has_value()) {
        try {
            resource_data = File::open_file(*resource_options.concatenate_against).value();
            concatenate_resource = load_resource_map(resource_data.data(), resource_data.size());

            if(reinterpret_cast<ResourceMapHeader *>(resource_data.data())->type != header.type) {
                eprintf_error("Cannot concatenate against a different resource map type than what is being made");
                return EXIT_FAILURE;
            }

            if(*resource_options.type == ResourceMapType::RESOURCE_MAP_LOC) {
                eprintf_error("Cannot concatenate against a loc resource map");
                return EXIT_FAILURE;
            }
        }
        catch (std::exception &) {
            eprintf_error("Failed to load %s", resource_options.concatenate_against->string().c_str());
            return EXIT_FAILURE;
        }
    }

    std::vector<std::size_t> offsets;
    std::vector<std::size_t> sizes;
    std::vector<std::string> paths;
    std::vector<std::string> added_tags;

    for(auto &listed_tag : tags_list) {
        // First let's open it
        TagFourCC tag_fourcc = TagFourCC::TAG_FOURCC_NONE;
        std::vector<std::byte> tag_data;

        // But if we don't know the extension, we need to find that first!
        if(listed_tag.fourcc == TagFourCC::TAG_FOURCC_NONE) {
            auto pref_path = File::halo_path_to_preferred_path(listed_tag.path.c_str());
            for(auto &tags_folder : resource_options.tags) {
                if(std::filesystem::exists(std::filesystem::path(tags_folder) / (pref_path + ".font"))) {
                    listed_tag.fourcc = TagFourCC::TAG_FOURCC_FONT;
                    break;
                }
                else if(std::filesystem::exists(std::filesystem::path(tags_folder) / (pref_path + ".hud_message_text"))) {
                    listed_tag.fourcc = TagFourCC::TAG_FOURCC_HUD_MESSAGE_TEXT;
                    break;
                }
                else if(std::filesystem::exists(std::filesystem::path(tags_folder) / (pref_path + ".unicode_string_list"))) {
                    listed_tag.fourcc = TagFourCC::TAG_FOURCC_UNICODE_STRING_LIST;
                    break;
                }
            }
            if(listed_tag.fourcc == TagFourCC::TAG_FOURCC_NONE) {
                eprintf_error("No font, hud message text, or unicode string list was found at %s.", pref_path.c_str());
                return EXIT_FAILURE;
            }
        }

        // Now, then!
        auto tag_path = File::halo_path_to_preferred_path(listed_tag.join());
        auto halo_tag_path = File::preferred_path_to_halo_path(listed_tag.path.c_str());

        // Did we add it?
        bool duplicate = false;
        for(auto &i : added_tags) {
            if(i == tag_path) {
                duplicate = true;
                break;
            }
        }
        if(duplicate) {
            continue;
        }
        added_tags.emplace_back(tag_path);

        switch(*resource_options.type) {
            case ResourceMapType::RESOURCE_MAP_BITMAP:
                tag_fourcc = TagFourCC::TAG_FOURCC_BITMAP;
                break;
            case ResourceMapType::RESOURCE_MAP_SOUND:
                tag_fourcc = TagFourCC::TAG_FOURCC_SOUND;
                break;
            case ResourceMapType::RESOURCE_MAP_LOC:
                switch(listed_tag.fourcc) {
                    case TagFourCC::TAG_FOURCC_FONT:
                    case TagFourCC::TAG_FOURCC_HUD_MESSAGE_TEXT:
                    case TagFourCC::TAG_FOURCC_UNICODE_STRING_LIST:
                        tag_fourcc = listed_tag.fourcc;
                        break;

                    // Okay, we didn't find anything
                    default:
                        eprintf_error("Expected a font, hud message text, or unicode string list. Got %s instead.", tag_fourcc_to_extension(listed_tag.fourcc));
                        return EXIT_FAILURE;
                }
                break;
        }

        if(tag_fourcc != listed_tag.fourcc) {
            eprintf_error("Expected %s. Got %s instead.", tag_fourcc_to_extension(tag_fourcc), tag_fourcc_to_extension(listed_tag.fourcc));
        }

        for(auto &tags_folder : resource_options.tags) {
            auto tag_path_path = std::filesystem::path(tags_folder) / tag_path;
            tag_data = Invader::File::open_file(tag_path_path).value_or(std::vector<std::byte>());
            if(tag_data.size()) {
                break;
            }
        }

        if(tag_data.size() == 0) {
            eprintf_error("Failed to open %s.", tag_path.c_str());
        }

        // This may be needed
        #define PAD_RESOURCES_32_BIT resource_data.insert(resource_data.end(), REQUIRED_PADDING_32_BIT(resource_data.size()), std::byte());

        // Compile the tags
        try {
            auto compiled_tag = Invader::BuildWorkload::compile_single_tag(listed_tag.path.c_str(), tag_fourcc, resource_options.tags);
            auto &compiled_tag_tag = compiled_tag.tags[0];
            auto &compiled_tag_struct = compiled_tag.structs[*compiled_tag_tag.base_struct];
            auto *compiled_tag_data = compiled_tag_struct.data.data();
            char path_temp[256];

            std::vector<std::byte> data;
            std::vector<std::size_t> structs;

            // Pointers are stored as offsets here
            auto write_pointers = [&data, &structs, &compiled_tag, &resource_options]() {
                for(auto &s : compiled_tag.structs) {
                    structs.push_back(data.size());
                    data.insert(data.end(), s.data.begin(), s.data.end());
                }

                std::size_t offset = resource_options.type == ResourceMapType::RESOURCE_MAP_SOUND ? sizeof(Invader::Parser::Sound::struct_little) : 0;
                for(auto &s : compiled_tag.structs) {
                    for(auto &ptr : s.pointers) {
                        *reinterpret_cast<LittleEndian<Pointer> *>(data.data() + structs[&s - compiled_tag.structs.data()] + ptr.offset) = structs[ptr.struct_index] - offset;
                    }
                }
            };

            // Now, adjust stuff for pointers
            switch(*resource_options.type) {
                case ResourceMapType::RESOURCE_MAP_BITMAP: {
                    // Do stuff to the tag data
                    auto &bitmap = *reinterpret_cast<Bitmap<LittleEndian> *>(compiled_tag_data);
                    std::size_t bitmap_count = bitmap.bitmap_data.count;

                    // Combine all data into one blob if custom edition
                    auto bitmap_data_offset_custom = resource_data.size();
                    if(resource_options.engine_target == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION) {
                        bool append = true;
                        std::vector<std::byte> data_custom;

                        for(auto &r : compiled_tag.raw_data) {
                            data_custom.insert(data_custom.end(), r.begin(), r.end());
                        }

                        for(auto &i : concatenate_resource) {
                            if(i.data == data_custom) {
                                bitmap_data_offset_custom = i.data_offset;
                                append = false;
                                if(resource_options.show_matched) {
                                    std::printf("Matched %s\n", File::halo_path_to_preferred_path(halo_tag_path).c_str());
                                }
                                break;
                            }
                        }

                        offsets.push_back(bitmap_data_offset_custom);
                        paths.push_back(halo_tag_path + "__pixels");
                        sizes.push_back(data_custom.size());

                        if(append) {
                            resource_data.insert(resource_data.end(), data_custom.begin(), data_custom.end());
                            PAD_RESOURCES_32_BIT
                        }
                    }

                    if(bitmap_count) {
                        auto *bitmaps = reinterpret_cast<BitmapData<LittleEndian> *>(compiled_tag.structs[*compiled_tag_struct.resolve_pointer(&bitmap.bitmap_data.pointer)].data.data());
                        for(std::size_t b = 0; b < bitmap_count; b++) {
                            auto *bitmap_data = bitmaps + b;

                            // If we're on retail, push the pixel data
                            if(retail) {
                                // Generate the path to add
                                std::snprintf(path_temp, sizeof(path_temp), "%s_%zu", halo_tag_path.c_str(), b);

                                // We already have it
                                for(auto &i : concatenate_resource) {
                                    if(i.data == compiled_tag.raw_data[b]) {
                                        paths.push_back(path_temp);
                                        sizes.push_back(i.data.size());
                                        offsets.push_back(i.data_offset);
                                        if(resource_options.show_matched) {
                                            std::printf("Matched %s\n", File::halo_path_to_preferred_path(path_temp).c_str());
                                        }
                                        goto next_bitmap_data;
                                    }
                                }

                                // Push it good
                                paths.push_back(path_temp);
                                std::size_t size = bitmap_data->pixel_data_size.read();
                                sizes.push_back(size);
                                offsets.push_back(resource_data.size());
                                resource_data.insert(resource_data.end(), compiled_tag.raw_data[b].begin(), compiled_tag.raw_data[b].end());

                                PAD_RESOURCES_32_BIT
                            }
                            // Otherwise set the sizes
                            else {
                                bitmap_data->pixel_data_offset = bitmap_data_offset_custom + bitmap_data->pixel_data_offset;
                                bitmap_data->flags = bitmap_data->flags.read() | BitmapDataFlagsFlag::BITMAP_DATA_FLAGS_FLAG_EXTERNAL;
                            }

                            next_bitmap_data: continue;
                        }
                    }

                    // Push the asset data and tag data if we aren't on retail
                    if(!retail) {
                        write_pointers();

                        // Push the tag data
                        offsets.push_back(resource_data.size());
                        resource_data.insert(resource_data.end(), data.begin(), data.end());
                        paths.push_back(halo_tag_path);
                        sizes.push_back(data.size());

                        PAD_RESOURCES_32_BIT
                    }

                    break;
                }
                case ResourceMapType::RESOURCE_MAP_SOUND: {
                    // Do stuff to the tag data
                    auto &sound = *reinterpret_cast<Sound<LittleEndian> *>(compiled_tag_struct.data.data());
                    std::size_t pitch_range_count = sound.pitch_ranges.count;
                    std::size_t b = 0;
                    std::size_t expected_offset = 0;

                    // Combine all data into one blob if custom edition
                    auto sound_data_offset_custom = resource_data.size();
                    if(resource_options.engine_target == HEK::CacheFileEngine::CACHE_FILE_CUSTOM_EDITION) {
                        bool append = true;
                        std::vector<std::byte> data_custom;

                        for(auto &r : compiled_tag.raw_data) {
                            data_custom.insert(data_custom.end(), r.begin(), r.end());
                        }

                        for(auto &i : concatenate_resource) {
                            if(i.data == data_custom) {
                                sound_data_offset_custom = i.data_offset;
                                append = false;
                                if(resource_options.show_matched) {
                                    std::printf("Matched %s\n", File::halo_path_to_preferred_path(halo_tag_path).c_str());
                                }
                                break;
                            }
                        }

                        offsets.push_back(sound_data_offset_custom);
                        paths.push_back(halo_tag_path + "__samples");
                        sizes.push_back(data_custom.size());

                        if(append) {
                            resource_data.insert(resource_data.end(), data_custom.begin(), data_custom.end());
                            PAD_RESOURCES_32_BIT
                        }
                    }

                    if(pitch_range_count) {
                        auto &pitch_range_struct = compiled_tag.structs[*compiled_tag_struct.resolve_pointer(&sound.pitch_ranges.pointer)];
                        auto *pitch_ranges = reinterpret_cast<SoundPitchRange<LittleEndian> *>(pitch_range_struct.data.data());
                        for(std::size_t pr = 0; pr < pitch_range_count; pr++) {
                            auto &pitch_range = pitch_ranges[pr];
                            std::size_t permutation_count = pitch_range.permutations.count;
                            if(permutation_count) {
                                auto *permutations = reinterpret_cast<SoundPermutation<LittleEndian> *>(compiled_tag.structs[*pitch_range_struct.resolve_pointer(&pitch_range.permutations.pointer)].data.data());
                                for(std::size_t p = 0; p < permutation_count; p++) {
                                    auto &permutation = permutations[p];
                                    if(retail) {
                                        // Generate the path to add
                                        std::snprintf(path_temp, sizeof(path_temp), "%s__%zu__%zu", halo_tag_path.c_str(), pr, p);

                                        // We already have it
                                        for(auto &i : concatenate_resource) {
                                            if(i.data == compiled_tag.raw_data[b]) {
                                                paths.push_back(path_temp);
                                                sizes.push_back(i.data.size());
                                                offsets.push_back(i.data_offset);
                                                if(resource_options.show_matched) {
                                                    std::printf("Matched %s\n", File::halo_path_to_preferred_path(path_temp).c_str());
                                                }
                                                goto next_sound_data;
                                            }
                                        }

                                        // Push it REAL good
                                        paths.push_back(path_temp);
                                        sizes.push_back(permutation.samples.size.read());
                                        offsets.push_back(resource_data.size());
                                        resource_data.insert(resource_data.end(), compiled_tag.raw_data[b].begin(), compiled_tag.raw_data[b].end());

                                        PAD_RESOURCES_32_BIT

                                        next_sound_data: b++;
                                    }
                                    else {
                                        permutation.samples.external = 1;
                                        permutation.samples.file_offset = sound_data_offset_custom + expected_offset;
                                        expected_offset += permutation.samples.size;
                                    }
                                }
                            }
                        }
                    }

                    // If we're not on retail, push asset and tag data
                    if(!retail) {
                        write_pointers();

                        // Push the tag data
                        offsets.push_back(resource_data.size());
                        resource_data.insert(resource_data.end(), data.begin(), data.end());
                        paths.push_back(halo_tag_path);
                        sizes.push_back(data.size());

                        PAD_RESOURCES_32_BIT
                    }

                    break;
                }
                case ResourceMapType::RESOURCE_MAP_LOC: {
                    write_pointers();
                    offsets.push_back(resource_data.size());
                    resource_data.insert(resource_data.end(), data.begin(), data.end());
                    paths.push_back(halo_tag_path);
                    sizes.push_back(data.size());

                    PAD_RESOURCES_32_BIT

                    break;
                }
            }
        }
        catch(std::exception &e) {
            eprintf_error("Failed to compile %s due to an exception: %s", tag_path.c_str(), e.what());
            return EXIT_FAILURE;
        }

        #undef PAD_RESOURCES_32_BIT
    }

    // Get the final path of the map
    const char *map;
    switch(*resource_options.type) {
        case ResourceMapType::RESOURCE_MAP_BITMAP:
            map = "bitmaps.map";
            break;
        case ResourceMapType::RESOURCE_MAP_SOUND:
            map = "sounds.map";
            break;
        case ResourceMapType::RESOURCE_MAP_LOC:
            map = "loc.map";
            break;
        default:
            std::terminate();
    }
    auto map_path = std::filesystem::path(resource_options.maps) / map;

    // Finish up building up the map
    std::size_t resource_count = paths.size();
    assert(resource_count == offsets.size());
    assert(resource_count == sizes.size());
    std::vector<ResourceMapResource> resource_indices(resource_count);
    std::vector<std::byte> resource_names_arr;
    for(std::size_t i = 0; i < resource_count; i++) {
        auto &index = resource_indices[i];
        index.size = sizes[i];
        index.data_offset = offsets[i];
        index.path_offset = resource_names_arr.size();
        resource_names_arr.insert(resource_names_arr.end(), reinterpret_cast<const std::byte *>(paths[i].c_str()), reinterpret_cast<const std::byte *>(paths[i].c_str()) + paths[i].size() + 1);
    }
    header.resource_count = resource_count;
    header.paths = resource_data.size();
    header.resources = resource_names_arr.size() + resource_data.size();
    *reinterpret_cast<ResourceMapHeader *>(resource_data.data()) = header;

    if(resource_data.size() >= 0xFFFFFFFF) {
        eprintf_error("Resource map exceeds 4 GiB.");
        return EXIT_FAILURE;
    }

    // Open the file
    std::FILE *f = std::fopen(map_path.string().c_str(), "wb");
    if(!f) {
        eprintf_error("Failed to open %s for writing.", map_path.string().c_str());
        return EXIT_FAILURE;
    }

    // Write everything
    std::fwrite(resource_data.data(), resource_data.size(), 1, f);
    std::fwrite(resource_names_arr.data(), resource_names_arr.size(), 1, f);
    std::fwrite(resource_indices.data(), resource_indices.size() * sizeof(*resource_indices.data()), 1, f);

    // Done!
    std::fclose(f);

    oprintf("Created a resource map file at %s\n", map_path.string().c_str());
}
