/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <filesystem>
#include <getopt.h>
#include "../tag/compiled_tag.hpp"
#include "../version.hpp"
#include "../tag/hek/class/bitmap.hpp"
#include "../tag/hek/class/sound.hpp"
#include "resource_map.hpp"
#include "hek/resource_map.hpp"
#include "list/resource_list.hpp"

int main(int argc, char *argv[]) {
    using namespace Invader::HEK;
    using namespace Invader;

    // Long options
    int longindex = 0;
    static struct option options[] = {
        {"info", no_argument, 0, 'i'},
        {"help", no_argument, 0, 'h'},
        {"type", required_argument, 0, 'T' },
        {"tags", required_argument, 0, 't' },
        {"maps", required_argument, 0, 'm' },
        {0, 0, 0, 0 }
    };

    // Tags directory
    std::vector<const char *> tags;

    // Maps directory
    const char *maps = "maps/";

    // Resource map type
    ResourceMapType type = ResourceMapType::RESOURCE_MAP_BITMAP;
    const char **(*default_fn)() = get_default_bitmap_resources;
    bool resource_map_set = false;

    int opt;

    // Go through each argument
    while((opt = getopt_long(argc, argv, "hit:T:m:", options, &longindex)) != -1) {
        switch(opt) {
            case 'i':
                INVADER_SHOW_INFO
                return EXIT_FAILURE;

            case 't':
                tags.push_back(optarg);
                break;

            case 'm':
                maps = optarg;
                break;

            case 'T':
                if(std::strcmp(optarg, "bitmaps") == 0) {
                    type = ResourceMapType::RESOURCE_MAP_BITMAP;
                    default_fn = get_default_bitmap_resources;
                }
                else if(std::strcmp(optarg, "sounds") == 0) {
                    type = ResourceMapType::RESOURCE_MAP_SOUND;
                    default_fn = get_default_sound_resources;
                }
                else if(std::strcmp(optarg, "loc") == 0) {
                    type = ResourceMapType::RESOURCE_MAP_LOC;
                    default_fn = get_default_loc_resources;
                }
                else {
                    eprintf("Invalid type %s. Use --help for more information.\n", optarg);
                    return EXIT_FAILURE;
                }
                resource_map_set = true;
                break;

            default:
                eprintf("Usage: %s <options>\n\n", *argv);
                eprintf("Create or modify a bitmap tag.\n\n");
                eprintf("Options:\n");
                eprintf("    --info,-i                  Show license and credits.\n");
                eprintf("    --help,-h                  Show help\n\n");
                eprintf("Directory options:\n");
                eprintf("    --maps,-m <path>           Set the maps directory.\n");
                eprintf("    --tags,-t <path>           Set the tags directory. Use multiple times to use\n");
                eprintf("                               multiple directories in order of precedence.\n\n");
                eprintf("Resource options:\n");
                eprintf("    --type,-T <type>           Set the resource map. This option is required for\n");
                eprintf("                               creating maps. Can be: bitmaps, sounds, or loc.\n\n");
                return EXIT_FAILURE;
        }
    }

    if(optind < argc) {
        eprintf("%s: Unexpected argument: %s\n", argv[0], argv[optind]);
        return EXIT_FAILURE;
    }

    if(!resource_map_set) {
        eprintf("No resource map type was given. Use --help for more information.\n");
        return EXIT_FAILURE;
    }

    // If we don't have any tags directories, use a default one
    if(tags.size() == 0) {
        tags.push_back("tags/");
    }

    // Get all the tags
    std::vector<std::string> tags_list;
    for(const char **i = default_fn(); *i; i++) {
        tags_list.insert(tags_list.end(), *i);
    }

    ResourceMapHeader header = {};
    header.type = type;
    header.resource_count = tags_list.size();

    // Read the amazing fun happy stuff
    std::vector<std::byte> resource_data(sizeof(ResourceMapHeader));
    std::vector<std::size_t> offsets;
    std::vector<std::size_t> sizes;
    std::vector<std::string> paths;

    for(const std::string &tag : tags_list) {
        // First let's open it
        TagClassInt tag_class_int;
        std::vector<std::byte> tag_data;

        // Convert backslashes if needed
        std::string tag_path = tag;
        for(char &c : tag_path) {
            if(c == '\\') {
                c = std::filesystem::path::preferred_separator;
            }
        }

        // Function to open a file
        auto open_tag = [&tags, &tag_path](const char *extension) -> std::FILE * {
            for(auto &tags_folder : tags) {
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

        switch(type) {
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

            // Now, adjust stuff for pointers
            switch(type) {
                case ResourceMapType::RESOURCE_MAP_BITMAP: {
                    // Pointers are stored as offsets here
                    for(auto &ptr : compiled_tag.pointers) {
                        *reinterpret_cast<LittleEndian<Pointer> *>(compiled_tag.data.data() + ptr.offset) = static_cast<Pointer>(ptr.offset_pointed);
                    }

                    // Push the asset data first
                    std::size_t bitmap_data_offset = resource_data.size();
                    offsets.push_back(bitmap_data_offset);
                    resource_data.insert(resource_data.end(), compiled_tag.asset_data.begin(), compiled_tag.asset_data.end());
                    paths.push_back(tag + "__pixels");
                    sizes.push_back(compiled_tag.asset_data.size());

                    PAD_RESOURCES_32_BIT

                    // Do stuff to the tag data
                    auto &bitmap = *reinterpret_cast<Bitmap<LittleEndian> *>(compiled_tag.data.data());
                    std::size_t bitmap_count = bitmap.bitmap_data.count;
                    if(bitmap_count) {
                        auto *bitmaps = reinterpret_cast<BitmapData<LittleEndian> *>(compiled_tag.data.data() + compiled_tag.resolve_pointer(&bitmap.bitmap_data.pointer));
                        auto *bitmaps_end = bitmaps + bitmap_count;
                        for(auto *bitmap = bitmaps; bitmap < bitmaps_end; bitmap++) {
                            auto flags = bitmap->flags.read();
                            flags.external = 1;
                            bitmap->flags = flags;
                            bitmap->pixels_offset = bitmap_data_offset + bitmap->pixels_offset;
                        }
                    }

                    // Push the tag data
                    offsets.push_back(resource_data.size());
                    resource_data.insert(resource_data.end(), compiled_tag.data.begin(), compiled_tag.data.end());
                    paths.push_back(tag);
                    sizes.push_back(compiled_tag.data.size());

                    PAD_RESOURCES_32_BIT

                    break;
                }
                case ResourceMapType::RESOURCE_MAP_SOUND: {
                    // Sounds subtract the size of the header from the offset
                    for(auto &ptr : compiled_tag.pointers) {
                        *reinterpret_cast<LittleEndian<Pointer> *>(compiled_tag.data.data() + ptr.offset) = static_cast<Pointer>(ptr.offset_pointed - sizeof(Sound<LittleEndian>));
                    }

                    // Push the asset data first
                    std::size_t bitmap_data_offset = resource_data.size();
                    offsets.push_back(bitmap_data_offset);
                    resource_data.insert(resource_data.end(), compiled_tag.asset_data.begin(), compiled_tag.asset_data.end());
                    paths.push_back(tag + "__permutations");
                    sizes.push_back(compiled_tag.asset_data.size());

                    PAD_RESOURCES_32_BIT

                    // Do stuff to the tag data
                    auto &sound = *reinterpret_cast<Sound<LittleEndian> *>(compiled_tag.data.data());
                    std::size_t pitch_range_count = sound.pitch_ranges.count;
                    if(pitch_range_count) {
                        auto *pitch_ranges = reinterpret_cast<SoundPitchRange<LittleEndian> *>(compiled_tag.data.data() + compiled_tag.resolve_pointer(&sound.pitch_ranges.pointer));
                        auto *pitch_ranges_end = pitch_ranges + pitch_range_count;
                        for(auto *pitch_range = pitch_ranges; pitch_range < pitch_ranges_end; pitch_range++) {
                            std::size_t permutation_count = pitch_range->permutations.count;
                            if(permutation_count) {
                                auto *permutations = reinterpret_cast<SoundPermutation<LittleEndian> *>(compiled_tag.data.data() + compiled_tag.resolve_pointer(&pitch_range->permutations.pointer));
                                auto *permutations_end = permutations + permutation_count;
                                for(auto *permutation = permutations; permutation < permutations_end; permutation++) {
                                    permutation->samples.external = 1;
                                    permutation->samples.file_offset = bitmap_data_offset + permutation->samples.file_offset;
                                }
                            }
                        }
                    }

                    // Push the tag data
                    offsets.push_back(resource_data.size());
                    resource_data.insert(resource_data.end(), compiled_tag.data.begin(), compiled_tag.data.end());
                    paths.push_back(tag);
                    sizes.push_back(compiled_tag.data.size());

                    PAD_RESOURCES_32_BIT

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
    switch(type) {
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
    auto map_path = std::filesystem::path(maps) / map;

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

    if(resource_data.size() >= 0x100000000) {
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
