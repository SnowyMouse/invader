// SPDX-License-Identifier: GPL-3.0-only

#include <zstd.h>
#include <zlib.h>
#include <tiffio.h>
#include <invader/file/file.hpp>
#include <invader/tag/parser/parser_struct.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/model/jms.hpp>
#include <invader/tag/parser/compile/string_list.hpp>
#include "recover_method.hpp"

namespace Invader::Recover {
    static void create_directories_for_path(const std::filesystem::path &path) {
        std::error_code ec;
        std::filesystem::create_directories(path.parent_path());
    }

    [[noreturn]] static void create_directories_save_and_quit(const std::filesystem::path &path, const std::vector<std::byte> &data) {
        create_directories_for_path(path);

        // Save it
        if(!File::save_file(path, data)) {
            eprintf_error("Failed to write to %s", path.string().c_str());
            std::exit(EXIT_FAILURE);
        }

        oprintf_success("Recovered %s", path.string().c_str());
        std::exit(EXIT_SUCCESS);
    }

    static void recover_tag_collection(const Parser::ParserStruct &tag, const std::string &path, const std::filesystem::path &data, bool overwrite) {
        auto *tag_collection = dynamic_cast<const Parser::TagCollection *>(&tag);
        if(!tag_collection) {
            return;
        }

        // Output it
        std::string output;
        for(auto &i : tag_collection->tags) {
            output += i.reference.path + "." + HEK::tag_fourcc_to_extension(i.reference.tag_fourcc) + "\n";
        }

        // Create directories
        auto file_path = data / (path + ".txt");

        if(std::filesystem::exists(file_path) && !overwrite) {
            eprintf("Skipped %s\n", file_path.string().c_str());
            std::exit(EXIT_SUCCESS);
        }

        auto *output_data = output.data();
        create_directories_save_and_quit(file_path, std::vector<std::byte>(reinterpret_cast<const std::byte *>(output_data), reinterpret_cast<const std::byte *>(output_data + output.size())));
    }

    static void recover_bitmap(const Parser::ParserStruct &tag, const std::string &path, const std::filesystem::path &data, bool overwrite) {
        auto *bitmap = dynamic_cast<const Parser::Bitmap *>(&tag);
        auto file_path = data / (path + ".tif");

        const std::vector<std::byte> *input;
        std::size_t width, height;
        if(bitmap) {
            input = &bitmap->compressed_color_plate_data;
            width = bitmap->color_plate_width;
            height = bitmap->color_plate_height;
        }
        else {
            return;
        }

        // Do we have color plate data?
        if(width == 0 || height == 0 || input->size() <= sizeof(HEK::BigEndian<std::uint32_t>)) {
            eprintf_warn("No color plate data to recover from - tag likely extracted");
            std::exit(EXIT_FAILURE);
        }

        // Does it already exist?
        if(std::filesystem::exists(file_path) && !overwrite) {
            eprintf("Skipped %s\n", file_path.string().c_str());
            std::exit(EXIT_SUCCESS);
        }

        // Get the size of the data we're going to decompress
        const auto *color_plate_data = input->data();
        auto image_size = reinterpret_cast<const HEK::BigEndian<std::uint32_t> *>(color_plate_data)->read();
        const auto *compressed_data = color_plate_data + sizeof(image_size);
        auto compressed_size = input->size() - sizeof(image_size);

        if(image_size != width * height * sizeof(std::uint32_t)) {
            eprintf_error("Color plate size is wrong");
            std::exit(EXIT_FAILURE);
        }
        std::vector<HEK::LittleEndian<std::uint32_t>> pixels(image_size);

        // Inflate if regular
        if(bitmap) {
            z_stream inflate_stream;
            inflate_stream.zalloc = Z_NULL;
            inflate_stream.zfree = Z_NULL;
            inflate_stream.opaque = Z_NULL;
            inflate_stream.avail_out = image_size;
            inflate_stream.next_out = reinterpret_cast<Bytef *>(pixels.data());
            inflate_stream.avail_in = compressed_size;
            inflate_stream.next_in = reinterpret_cast<Bytef *>(const_cast<std::byte *>(compressed_data));

            // Do it
            if(inflateInit(&inflate_stream) != Z_OK || inflate(&inflate_stream, Z_FINISH) != Z_OK || inflateEnd(&inflate_stream) != Z_OK) {
                eprintf_error("Color plate data could not be decompressed");
                std::exit(EXIT_FAILURE);
            }
        }

        // Let's begin
        auto file_path_str = file_path.string();
        create_directories_for_path(file_path);

        auto *tiff = TIFFOpen(file_path_str.c_str(), "w");
        if(!tiff) {
            eprintf_error("Failed to open %s for writing", file_path_str.c_str());
            std::exit(EXIT_FAILURE);
        }

        TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, width);
        TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, height);

        std::uint16_t extrasamples = EXTRASAMPLE_UNASSALPHA;
        TIFFSetField(tiff, TIFFTAG_EXTRASAMPLES, 1, &extrasamples);

        TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 4);
        TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 8, 8, 8, 8);
        TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, 1);
        TIFFSetField(tiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
        TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
        TIFFSetField(tiff, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
        TIFFSetField(tiff, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

        // Swap blue and red channels
        for(auto &i : pixels) {
            i = (i & 0xFF00FF00) | ((i & 0x00FF0000) >> 16) | ((i & 0x000000FF) << 16);
        }

        // Write it
        for (std::size_t y = 0; y < height; y++) {
            TIFFWriteScanline(tiff, pixels.data() + y * width, y, 0);
        }

        TIFFClose(tiff);

        oprintf_success("Recovered %s", file_path_str.c_str());

        std::exit(EXIT_SUCCESS);
    }

    static const char *ALL_LODS[] = {
        "superhigh",
        "high",
        "medium",
        "low",
        "superlow"
    };

    template<typename T> static void make_jms(const T &model, const std::string &permutation, std::size_t lod, const std::filesystem::path &models_path, bool local_nodes) {
        JMS jms;

        // Set the checksum value
        jms.node_list_checksum = model.node_list_checksum;

        // Fill out the nodes
        for(const auto &node : model.nodes) {
            auto &new_node = jms.nodes.emplace_back();
            new_node.name = node.name.string;
            new_node.position = node.default_translation;
            new_node.rotation = node.default_rotation;
            new_node.sibling_node = node.next_sibling_node_index;
            new_node.first_child = node.first_child_node_index;
        }

        // Map a shader indices from the model to the JMS
        std::map<std::string, std::optional<std::size_t>> shader_map;

        float u_scale = model.base_map_u_scale == 0.0F ? 1.0F : model.base_map_u_scale;
        float v_scale = model.base_map_v_scale == 0.0F ? 1.0F : model.base_map_v_scale;

        // Get the region
        for(auto &r : model.regions) {
            for(auto &p : r.permutations) {
                if(permutation == p.name.string) {
                    // Get the geometry index
                    std::size_t geometry_index;
                    bool can_be_used;
                    switch(lod) {
                        case 0:
                            geometry_index = p.super_high;
                            can_be_used = true;
                            break;
                        case 1:
                            geometry_index = p.high;
                            can_be_used = geometry_index != p.super_high;
                            break;
                        case 2:
                            geometry_index = p.medium;
                            can_be_used = geometry_index != p.high;
                            break;
                        case 3:
                            geometry_index = p.low;
                            can_be_used = geometry_index != p.medium;
                            break;
                        case 4:
                            geometry_index = p.super_low;
                            can_be_used = geometry_index != p.low;
                            break;
                        default:
                            std::terminate();
                    }

                    // Is it valid?
                    if(geometry_index >= model.geometries.size()) {
                        throw Invader::InvalidTagDataException();
                    }

                    // Skip it?
                    if(!can_be_used) {
                        continue;
                    }

                    // Add our new region
                    auto new_region_index = jms.regions.size();
                    auto &new_region = jms.regions.emplace_back();
                    new_region.name = r.name.string;

                    // If superhigh, export markers too
                    if(lod == 0) {
                        for(auto &m : p.markers) {
                            auto &marker = jms.markers.emplace_back();
                            marker.name = m.name.string;
                            marker.node = m.node_index;
                            marker.position = m.translation;
                            marker.rotation = m.rotation;
                            marker.region = new_region_index;
                        }
                    }

                    // Geometry
                    auto &geometry = model.geometries[geometry_index];

                    for(auto &p : geometry.parts) {
                        // Is the shader valid?
                        if(p.shader_index >= model.shaders.size()) {
                            eprintf_error("Invalid shader index!");
                            throw Invader::InvalidTagDataException();
                        }

                        // Here we go
                        auto &shader_entry = model.shaders[p.shader_index];
                        auto shader_name = std::filesystem::path(File::halo_path_to_preferred_path(shader_entry.shader.path)).filename().string();

                        // If the permutation is non-zero, also add the permutation index
                        if(shader_entry.permutation > 0) {
                            shader_name += std::to_string(shader_entry.permutation);
                        }

                        auto &mapped_index_maybe = shader_map[shader_name];

                        // Add the shader if not present
                        if(!mapped_index_maybe.has_value()) {
                            mapped_index_maybe = jms.materials.size();
                            auto &material = jms.materials.emplace_back();
                            material.name = shader_name;
                            material.tif_path = "<none>";
                        }

                        // Uncompressed vertices are missing
                        if(p.uncompressed_vertices.size() == 0) {
                            eprintf_error("Missing uncompressed vertices - tag likely extracted improperly (haw haw!)");
                            throw Invader::InvalidTagDataException();
                        }

                        // Begin extracting the geometry data
                        auto shader_mapped_index = *mapped_index_maybe;

                        // Extract the vertices
                        std::size_t first_vertex_offset = jms.vertices.size();
                        for(auto &v : p.uncompressed_vertices) {
                            auto &vertex = jms.vertices.emplace_back();

                            // Handle local nodes
                            if(local_nodes) {
                                auto *memes = reinterpret_cast<const Parser::GBXModelGeometryPart *>(&p);
                                if(memes->local_node_count > sizeof(memes->local_node_indices) / sizeof(*memes->local_node_indices)) {
                                    eprintf_error("Local nodes overflow");
                                    throw Invader::InvalidTagDataException();
                                }

                                if(v.node0_index != NULL_INDEX) {
                                    if(v.node0_index >= memes->local_node_count) {
                                        eprintf_error("Local nodes index out of bounds");
                                        throw Invader::InvalidTagDataException();
                                    }
                                    vertex.node0 = v.node0_index;
                                }
                                else {
                                    vertex.node0 = NULL_INDEX;
                                }

                                if(v.node1_index != NULL_INDEX) {
                                    if(v.node1_index >= memes->local_node_count) {
                                        eprintf_error("Local nodes index out of bounds");
                                        throw Invader::InvalidTagDataException();
                                    }
                                    vertex.node1 = v.node1_index;
                                }
                                else {
                                    vertex.node1 = NULL_INDEX;
                                }
                            }
                            else {
                                vertex.node0 = v.node0_index;
                                vertex.node1 = v.node1_index;
                            }

                            vertex.node1_weight = v.node1_weight;
                            vertex.normal = v.normal;
                            vertex.position = v.position;
                            vertex.texture_coordinates = v.texture_coords;

                            vertex.texture_coordinates.x = vertex.texture_coordinates.x * u_scale;
                            vertex.texture_coordinates.y = vertex.texture_coordinates.y * v_scale;
                        }

                        // Add indices
                        std::vector<HEK::Index> indices;
                        for(auto &t : p.triangles) {
                            indices.emplace_back(t.vertex0_index);
                            indices.emplace_back(t.vertex1_index);
                            indices.emplace_back(t.vertex2_index);
                        }

                        // Error if that went wrong
                        if(indices.size() < 3) {
                            eprintf_error("Geometry has no geometry");
                            throw Invader::InvalidTagDataException();
                        }

                        // Begin adding the triangles
                        auto triangle_count = indices.size() - 2;
                        auto *triangle = indices.data();
                        auto *triangle_end = indices.data() + triangle_count;
                        bool flipped_normal = false;
                        for(; triangle < triangle_end; triangle++) {
                            // Get the vertex indices
                            auto a = triangle[0];
                            auto b = triangle[flipped_normal ? 2 : 1];
                            auto c = triangle[flipped_normal ? 1 : 2];
                            flipped_normal = !flipped_normal;

                            // Check if degenerate or null. If so, skip
                            if(a == b || b == c || a == c || a == NULL_INDEX || b == NULL_INDEX || c == NULL_INDEX) {
                                continue;
                            }

                            // Read it!
                            auto &new_triangle = jms.triangles.emplace_back();
                            new_triangle.region = new_region_index;
                            new_triangle.shader = shader_mapped_index;
                            new_triangle.vertices[0] = first_vertex_offset + a;
                            new_triangle.vertices[1] = first_vertex_offset + b;
                            new_triangle.vertices[2] = first_vertex_offset + c;
                        }
                    }

                    break;
                }
            }
        }

        // If we didn't get anything, return
        if(jms.triangles.size() == 0) {
            return;
        }

        // Filename memery
        auto filename = models_path / (permutation + " " + ALL_LODS[lod] + ".jms");
        auto string_data = jms.string();
        auto *jms_data = string_data.data();
        if(File::save_file(filename, std::vector<std::byte>(reinterpret_cast<std::byte *>(jms_data), reinterpret_cast<std::byte *>(jms_data + string_data.size())))) {
            oprintf_success("Recovered %s", filename.string().c_str());
        }
        else {
            eprintf_error("Failed to write to %s", filename.string().c_str());
            std::exit(EXIT_FAILURE);
        }
    }

    template<typename T> static void recover_jms(const Parser::ParserStruct &tag, const std::string &path, const std::filesystem::path &data, bool overwrite) {
        auto *model = dynamic_cast<const T *>(&tag);
        if(!model) {
            return;
        }

        // Check if the parent filename is the same
        auto path_path = std::filesystem::path(path);
        auto parent_path_path = path_path.parent_path();
        if(parent_path_path.filename() != path_path.filename()) {
            eprintf_error("Cannot recover due to parent filename not matching tag filename");
            eprintf_error("Parent filename is %s, but the tag's filename is %s", parent_path_path.filename().string().c_str(), path_path.filename().string().c_str());
            std::exit(EXIT_FAILURE);
        }

        // Models
        auto model_directory = data / parent_path_path / "models";

        if(std::filesystem::exists(model_directory)) {
            if(!overwrite) {
                eprintf("Skipped %s\n", model_directory.string().c_str());
                std::exit(EXIT_SUCCESS);
            }
            else {
                std::error_code ec;
                std::filesystem::remove_all(model_directory);
                if(ec) {
                    eprintf_error("Cannot remove %s", model_directory.string().c_str());
                    std::exit(EXIT_FAILURE);
                }
            }
        }

        // Create directories if needed
        std::error_code ec;
        std::filesystem::create_directories(model_directory, ec);

        if(model->markers.size() > 0) {
            eprintf_error("Markers are present in base struct - tag likely extracted improperly (haw haw!)");
            throw Invader::InvalidTagDataException();
        }

        bool local_nodes = model->flags == HEK::ModelFlagsFlag::MODEL_FLAGS_FLAG_PARTS_HAVE_LOCAL_NODES;

        // Get all permutations
        std::vector<std::string> permutations;
        for(auto &r : model->regions) {
            for(auto &p : r.permutations) {
                bool in_it = false;
                for(auto &p2 : permutations) {
                    if(p2 == p.name.string) {
                        in_it = true;
                        break;
                    }
                }
                if(!in_it) {
                    permutations.emplace_back(p.name.string);
                }
            }
        }

        // Make all permutations
        for(auto &p : permutations) {
            for(std::size_t i = 0; i < sizeof(ALL_LODS) / sizeof(*ALL_LODS); i++) {
                make_jms(*model, p, i, model_directory, local_nodes);
            }
        }

        std::exit(EXIT_SUCCESS);
    }

    static void recover_string_list(const Parser::ParserStruct &tag, const std::string &path, const std::filesystem::path &data, bool overwrite) {
        auto *unicode_string_list = dynamic_cast<const Parser::UnicodeStringList *>(&tag);
        auto *string_list = dynamic_cast<const Parser::StringList *>(&tag);

        auto data_path = data / (path + ".txt");

        if(std::filesystem::exists(data_path) && !overwrite) {
            eprintf("Skipped %s\n", data_path.string().c_str());
            std::exit(EXIT_SUCCESS);
        }

        if(string_list) {
            std::string result;

            if(Parser::check_for_broken_strings(*string_list)) {
                eprintf_error("String list has broken strings - tag is corrupt or edited improperly");
                std::exit(EXIT_FAILURE);
            }

            // Just do it!
            for(auto &i : string_list->strings) {
                result += std::string(reinterpret_cast<const char *>(i.string.data())) + "\r\n###END-STRING###\r\n";
            }

            // Save
            auto *final_result = reinterpret_cast<const std::byte *>(result.data());
            create_directories_save_and_quit(data_path, std::vector<std::byte>(final_result, final_result + result.size()));
        }
        else if(unicode_string_list) {
            // Start with the BOM
            std::u16string result = u"\xFEFF";

            // Oh... also, is this broken?
            if(Parser::check_for_broken_strings(*unicode_string_list)) {
                eprintf_error("String list has broken strings - tag is corrupt or edited improperly");
                std::exit(EXIT_FAILURE);
            }

            // Go through each string and add it
            for(auto &i : unicode_string_list->strings) {
                result += reinterpret_cast<const char16_t *>(i.string.data());
                result += u"\r\n###END-STRING###\r\n";
            }

            // Save
            auto *result_data = result.data();
            auto *final_result = reinterpret_cast<const std::byte *>(result_data);
            auto *final_result_end = reinterpret_cast<const std::byte *>(result_data + result.size());
            create_directories_save_and_quit(data_path, std::vector<std::byte>(final_result, final_result_end));
        }
        else {
            return;
        }
    }

    static void recover_scripts(const Parser::ParserStruct &tag, const std::string &path, const std::filesystem::path &data, bool overwrite) {
        auto *scenario = dynamic_cast<const Parser::Scenario *>(&tag);
        if(!scenario) {
            return;
        }

        if(scenario->source_files.size() == 0) {
            eprintf_warn("Scenario does not have any script source data to recover");
            std::exit(EXIT_FAILURE);
        }

        // Models
        auto scripts_directory = data / std::filesystem::path(path).replace_extension() / "scripts";

        if(std::filesystem::exists(scripts_directory)) {
            if(!overwrite) {
                eprintf("Skipped %s\n", scripts_directory.string().c_str());
                std::exit(EXIT_SUCCESS);
            }
            else {
                std::error_code ec;
                std::filesystem::remove_all(scripts_directory);
                if(ec) {
                    eprintf_error("Cannot remove %s", scripts_directory.string().c_str());
                    std::exit(EXIT_FAILURE);
                }
            }
        }

        // Create directories if needed
        std::error_code ec;
        std::filesystem::create_directories(scripts_directory, ec);

        // Write it all
        for(auto &hsc : scenario->source_files) {
            auto hsc_path = scripts_directory / (std::string(hsc.name.string) + ".hsc");
            if(File::save_file(hsc_path, hsc.source)) {
                oprintf_success("Recovered %s", hsc_path.string().c_str());
            }
            else {
                eprintf_error("Failed to write to %s", hsc_path.string().c_str());
                std::exit(EXIT_FAILURE);
            }
        }

        std::exit(EXIT_SUCCESS);
    }

    void recover_model(const Parser::ParserStruct &tag, const std::string &path, const std::filesystem::path &data, bool overwrite) {
        recover_jms<Parser::Model>(tag, path, data, overwrite);
        recover_jms<Parser::GBXModel>(tag, path, data, overwrite);
    }

    void recover(const Parser::ParserStruct &tag, const std::string &path, const std::filesystem::path &data, HEK::TagFourCC tag_fourcc, bool overwrite) {
        recover_bitmap(tag, path, data, overwrite);
        recover_tag_collection(tag, path, data, overwrite);
        recover_model(tag, path, data, overwrite);
        recover_string_list(tag, path, data, overwrite);
        recover_scripts(tag, path, data, overwrite);

        eprintf_warn("Data cannot be recovered from tag class %s", HEK::tag_fourcc_to_extension(tag_fourcc));
        std::exit(EXIT_FAILURE);
    }
}
