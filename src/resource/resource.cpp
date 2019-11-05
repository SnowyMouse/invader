// SPDX-License-Identifier: GPL-3.0-only

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <filesystem>
#include <invader/tag/compiled_tag.hpp>
#include <invader/version.hpp>
#include <invader/tag/hek/class/bitmap.hpp>
#include <invader/tag/hek/class/sound.hpp>
#include <invader/resource/resource_map.hpp>
#include <invader/resource/hek/resource_map.hpp>
#include <invader/resource/list/resource_list.hpp>
#include <invader/command_line_option.hpp>
#include <invader/printf.hpp>

int main(int argc, const char **argv) {
    using namespace Invader::HEK;
    using namespace Invader;

    std::vector<CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("type", 'T', 1, "Set the resource map. This option is required for creating maps. Can be: bitmaps, sounds, or loc.", "<type>");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory. Use multiple times to add more directories, ordered by precedence.", "<dir>");
    options.emplace_back("maps", 'm', 1, "Set the maps directory.", "<dir>");
    options.emplace_back("retail", 'R', 0, "Build a retail resource map (bitmaps/sounds only)");

    static constexpr char DESCRIPTION[] = "Create resource maps.";
    static constexpr char USAGE[] = "<options>";

    struct ResourceOption {
        // Tags directory
        std::vector<const char *> tags;

        // Maps directory
        const char *maps = "maps/";

        // Resource map type
        ResourceMapType type = ResourceMapType::RESOURCE_MAP_BITMAP;
        const char **(*default_fn)() = get_default_bitmap_resources;
        bool resource_map_set = false;

        bool retail = false;
    } resource_options;

    auto remaining_arguments = CommandLineOption::parse_arguments<ResourceOption &>(argc, argv, options, USAGE, DESCRIPTION, 0, 0, resource_options, [](char opt, const std::vector<const char *> &arguments, auto &resource_options) {
        switch(opt) {
            case 'i':
                show_version_info();
                std::exit(EXIT_FAILURE);

            case 't':
                resource_options.tags.push_back(arguments[0]);
                break;

            case 'm':
                resource_options.maps = arguments[0];
                break;

            case 'R':
                resource_options.retail = true;
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
                    eprintf("Invalid type %s. Use --help for more information.\n", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                resource_options.resource_map_set = true;
                break;
        }
    });

    if(!resource_options.resource_map_set) {
        eprintf("No resource map type was given. Use -h for more information.\n");
        return EXIT_FAILURE;
    }

    if(resource_options.retail && resource_options.type == ResourceMapType::RESOURCE_MAP_LOC) {
        eprintf("Only bitmaps.map and sounds.map can be made for retail.\n");
        return EXIT_FAILURE;
    }

    // If we don't have any tags directories, use a default one
    if(resource_options.tags.size() == 0) {
        resource_options.tags.push_back("tags");
    }

    // Get all the tags
    std::vector<std::string> tags_list;
    for(const char **i = resource_options.default_fn(); *i; i++) {
        tags_list.insert(tags_list.end(), *i);
    }

    ResourceMapHeader header = {};
    header.type = resource_options.type;
    header.resource_count = tags_list.size();

    // Read the amazing fun happy stuff
    std::vector<std::byte> resource_data(sizeof(ResourceMapHeader));
    std::vector<std::size_t> offsets;
    std::vector<std::size_t> sizes;
    std::vector<std::string> paths;

    for(const std::string &tag : tags_list) {
        // First let's open it
        TagClassInt tag_class_int = TagClassInt::TAG_CLASS_NONE;
        std::vector<std::byte> tag_data;

        // Convert backslashes if needed
        std::string tag_path = tag;
        for(char &c : tag_path) {
            if(c == '\\') {
                c = std::filesystem::path::preferred_separator;
            }
        }

        // Function to open a file
        auto open_tag = [&resource_options, &tag_path](const char *extension) -> std::FILE * {
            for(auto &tags_folder : resource_options.tags) {
                auto tag_path_str = (std::filesystem::path(tags_folder) / tag_path).string() + extension;
                std::FILE *f = std::fopen(tag_path_str.data(), "rb");
                if(f) {
                    return f;
                }
            }
            return nullptr;
        };

        #define ATTEMPT_TO_OPEN(extension) { \
            std::FILE *f = open_tag(extension); \
            if(!f) { \
                eprintf("Failed to open %s" extension "\n", tag_path.data()); \
                return EXIT_FAILURE; \
            } \
            std::fseek(f, 0, SEEK_END); \
            tag_data.insert(tag_data.end(), std::ftell(f), std::byte()); \
            std::fseek(f, 0, SEEK_SET); \
            if(std::fread(tag_data.data(), tag_data.size(), 1, f) != 1) { \
                eprintf("Failed to read %s" extension "\n", tag_path.data()); \
                return EXIT_FAILURE; \
            } \
            std::fclose(f); \
        }

        switch(resource_options.type) {
            case ResourceMapType::RESOURCE_MAP_BITMAP:
                tag_class_int = TagClassInt::TAG_CLASS_BITMAP;
                ATTEMPT_TO_OPEN(".bitmap")
                break;
            case ResourceMapType::RESOURCE_MAP_SOUND:
                tag_class_int = TagClassInt::TAG_CLASS_SOUND;
                ATTEMPT_TO_OPEN(".sound")
                break;
            case ResourceMapType::RESOURCE_MAP_LOC:
                tag_class_int = TagClassInt::TAG_CLASS_FONT;
                #define DO_THIS_FOR_ME_PLEASE(tci, extension) if(tag_data.size() == 0) { \
                    tag_class_int = tci; \
                    std::FILE *f = open_tag(extension); \
                    if(f) { \
                        std::fseek(f, 0, SEEK_END); \
                        tag_data.insert(tag_data.end(), std::ftell(f), std::byte()); \
                        std::fseek(f, 0, SEEK_SET); \
                        if(std::fread(tag_data.data(), tag_data.size(), 1, f) != 1) { \
                            eprintf("Failed to read %s" extension "\n", tag_path.data()); \
                            return EXIT_FAILURE; \
                        } \
                        std::fclose(f); \
                    } \
                }
                DO_THIS_FOR_ME_PLEASE(TagClassInt::TAG_CLASS_FONT, ".font")
                DO_THIS_FOR_ME_PLEASE(TagClassInt::TAG_CLASS_HUD_MESSAGE_TEXT, ".hud_message_text")
                DO_THIS_FOR_ME_PLEASE(TagClassInt::TAG_CLASS_UNICODE_STRING_LIST, ".unicode_string_list")
                #undef DO_THIS_FOR_ME_PLEASE

                if(tag_data.size() == 0) {
                    eprintf("Failed to open %s.\nNo such font, hud_message_text, or unicode_string_list were found.\n", tag_path.data());
                }

                break;
        }

        #undef ATTEMPT_TO_OPEN

        // This may be needed
        #define PAD_RESOURCES_32_BIT resource_data.insert(resource_data.end(), REQUIRED_PADDING_32_BIT(resource_data.size()), std::byte());

        // Compile the tags
        try {
            CompiledTag compiled_tag(tag, tag_class_int, tag_data.data(), tag_data.size());
            char path_temp[256];

            // Now, adjust stuff for pointers
            switch(resource_options.type) {
                case ResourceMapType::RESOURCE_MAP_BITMAP: {
                    // Pointers are stored as offsets here
                    for(auto &ptr : compiled_tag.pointers) {
                        *reinterpret_cast<LittleEndian<Pointer> *>(compiled_tag.data.data() + ptr.offset) = static_cast<Pointer>(ptr.offset_pointed);
                    }

                    // Do stuff to the tag data
                    auto &bitmap = *reinterpret_cast<Bitmap<LittleEndian> *>(compiled_tag.data.data());
                    std::size_t bitmap_count = bitmap.bitmap_data.count;
                    if(bitmap_count) {
                        auto *bitmaps = reinterpret_cast<BitmapData<LittleEndian> *>(compiled_tag.data.data() + compiled_tag.resolve_pointer(&bitmap.bitmap_data.pointer));
                        for(std::size_t b = 0; b < bitmap_count; b++) {
                            auto *bitmap = bitmaps + b;

                            // If we're on retail, push the pixel data
                            if(resource_options.retail) {
                                // Generate the path to add
                                std::snprintf(path_temp, sizeof(path_temp), "%s_%zu", tag.data(), b);

                                // Push it good
                                paths.push_back(path_temp);
                                std::size_t size = bitmap->pixels_count.read();
                                std::size_t offset = bitmap->pixels_offset.read();
                                sizes.push_back(size);
                                offsets.push_back(resource_data.size());
                                resource_data.insert(resource_data.end(), compiled_tag.asset_data.data() + offset, compiled_tag.asset_data.data() + offset + size);

                                PAD_RESOURCES_32_BIT
                            }
                            // Otherwise set the sizes
                            else {
                                bitmap->pixels_offset = resource_data.size() + bitmap->pixels_offset;
                                auto flags = bitmap->flags.read();
                                flags.external = 1;
                                bitmap->flags = flags;
                            }
                        }
                    }

                    // Push the asset data and tag data if we aren't on retail
                    if(!resource_options.retail) {
                        offsets.push_back(resource_data.size());
                        resource_data.insert(resource_data.end(), compiled_tag.asset_data.begin(), compiled_tag.asset_data.end());
                        paths.push_back(tag + "__pixels");
                        sizes.push_back(compiled_tag.asset_data.size());

                        PAD_RESOURCES_32_BIT

                        // Push the tag data
                        offsets.push_back(resource_data.size());
                        resource_data.insert(resource_data.end(), compiled_tag.data.begin(), compiled_tag.data.end());
                        paths.push_back(tag);
                        sizes.push_back(compiled_tag.data.size());

                        PAD_RESOURCES_32_BIT
                    }

                    break;
                }
                case ResourceMapType::RESOURCE_MAP_SOUND: {
                    // Sounds subtract the size of the header from the offset
                    for(auto &ptr : compiled_tag.pointers) {
                        *reinterpret_cast<LittleEndian<Pointer> *>(compiled_tag.data.data() + ptr.offset) = static_cast<Pointer>(ptr.offset_pointed - sizeof(Sound<LittleEndian>));
                    }

                    // Do stuff to the tag data
                    auto &sound = *reinterpret_cast<Sound<LittleEndian> *>(compiled_tag.data.data());
                    std::size_t pitch_range_count = sound.pitch_ranges.count;
                    if(pitch_range_count) {
                        auto *pitch_ranges = reinterpret_cast<SoundPitchRange<LittleEndian> *>(compiled_tag.data.data() + compiled_tag.resolve_pointer(&sound.pitch_ranges.pointer));
                        for(std::size_t pr = 0; pr < pitch_range_count; pr++) {
                            auto *pitch_range = pitch_ranges + pr;
                            std::size_t permutation_count = pitch_range->permutations.count;
                            if(permutation_count) {
                                auto *permutations = reinterpret_cast<SoundPermutation<LittleEndian> *>(compiled_tag.data.data() + compiled_tag.resolve_pointer(&pitch_range->permutations.pointer));
                                for(std::size_t p = 0; p < permutation_count; p++) {
                                    auto *permutation = permutations + p;
                                    if(resource_options.retail) {
                                        // Generate the path to add
                                        std::snprintf(path_temp, sizeof(path_temp), "%s__%zu__%zu", tag.data(), pr, p);

                                        // Push it REAL good
                                        paths.push_back(path_temp);
                                        std::size_t size = permutation->samples.size.read();
                                        sizes.push_back(size);
                                        offsets.push_back(resource_data.size());
                                        std::size_t offset = permutation->samples.file_offset.read();
                                        resource_data.insert(resource_data.end(), compiled_tag.asset_data.data() + offset, compiled_tag.asset_data.data() + offset + size);

                                        PAD_RESOURCES_32_BIT
                                    }
                                    else {
                                        permutation->samples.external = 1;
                                        permutation->samples.file_offset = resource_data.size() + permutation->samples.file_offset;
                                    }
                                }
                            }
                        }
                    }

                    // If we're not on retail, push asset and tag data
                    if(!resource_options.retail) {
                        // Push the asset data first
                        offsets.push_back(resource_data.size());
                        resource_data.insert(resource_data.end(), compiled_tag.asset_data.begin(), compiled_tag.asset_data.end());
                        paths.push_back(tag + "__permutations");
                        sizes.push_back(compiled_tag.asset_data.size());

                        PAD_RESOURCES_32_BIT

                        // Push the tag data
                        offsets.push_back(resource_data.size());
                        resource_data.insert(resource_data.end(), compiled_tag.data.begin(), compiled_tag.data.end());
                        paths.push_back(tag);
                        sizes.push_back(compiled_tag.data.size());

                        PAD_RESOURCES_32_BIT
                    }

                    break;
                }
                case ResourceMapType::RESOURCE_MAP_LOC:
                    // Pointers are stored as offsets here
                    for(auto &ptr : compiled_tag.pointers) {
                        *reinterpret_cast<LittleEndian<Pointer> *>(compiled_tag.data.data() + ptr.offset) = static_cast<Pointer>(ptr.offset_pointed);
                    }
                    offsets.push_back(resource_data.size());
                    resource_data.insert(resource_data.end(), compiled_tag.data.begin(), compiled_tag.data.end());
                    paths.push_back(tag);
                    sizes.push_back(compiled_tag.data.size());

                    PAD_RESOURCES_32_BIT

                    break;
            }
        }
        catch(std::exception &e) {
            eprintf("Failed to compile %s.%s due to an exception: %s\n", tag_path.data(), tag_class_to_extension(tag_class_int), e.what());
            return EXIT_FAILURE;
        }

        #undef PAD_RESOURCES_32_BIT
    }

    // Get the final path of the map
    const char *map;
    switch(resource_options.type) {
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
        resource_names_arr.insert(resource_names_arr.end(), reinterpret_cast<std::byte *>(paths[i].data()), reinterpret_cast<std::byte *>(paths[i].data()) + paths[i].size() + 1);
    }
    header.resource_count = resource_count;
    header.paths = resource_data.size();
    header.resources = resource_names_arr.size() + resource_data.size();
    *reinterpret_cast<ResourceMapHeader *>(resource_data.data()) = header;

    if(resource_data.size() >= 0xFFFFFFFF) {
        eprintf("Resource map exceeds 4 GiB.\n");
        return EXIT_FAILURE;
    }

    // Open the file
    std::FILE *f = std::fopen(map_path.string().data(), "wb");
    if(!f) {
        eprintf("Failed to open %s for writing.\n", map_path.string().data());
        return EXIT_FAILURE;
    }

    // Write everything
    std::fwrite(resource_data.data(), resource_data.size(), 1, f);
    std::fwrite(resource_names_arr.data(), resource_names_arr.size(), 1, f);
    std::fwrite(resource_indices.data(), resource_indices.size() * sizeof(*resource_indices.data()), 1, f);

    // Done!
    std::fclose(f);
}
