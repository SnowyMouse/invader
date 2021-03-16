// SPDX-License-Identifier: GPL-3.0-only

#include <zstd.h>
#include <zlib.h>
#include <tiffio.h>
#include <invader/file/file.hpp>
#include <invader/tag/parser/parser_struct.hpp>
#include <invader/tag/parser/parser.hpp>
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
    
    static void recover_tag_collection(const Parser::ParserStruct &tag, const std::string &path, const std::filesystem::path &data) {
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
        
        auto *output_data = output.data();
        create_directories_save_and_quit(file_path, std::vector<std::byte>(reinterpret_cast<const std::byte *>(output_data), reinterpret_cast<const std::byte *>(output_data + output.size())));
    }
    
    static void recover_bitmap(const Parser::ParserStruct &tag, const std::string &path, const std::filesystem::path &data) {
        auto *bitmap = dynamic_cast<const Parser::Bitmap *>(&tag);
        auto *invader_bitmap = dynamic_cast<const Parser::InvaderBitmap *>(&tag);
        
        const std::vector<std::byte> *input;
        std::size_t width, height;
        if(bitmap) {
            input = &bitmap->compressed_color_plate_data;
            width = bitmap->color_plate_width;
            height = bitmap->color_plate_height;
        }
        else if(invader_bitmap) {
            input = &invader_bitmap->compressed_color_plate_data;
            width = bitmap->color_plate_width;
            height = bitmap->color_plate_height;
        }
        else {
            return;
        }
        
        // Do we have color palte data?
        if(width == 0 || height == 0 || input->size() <= sizeof(HEK::BigEndian<std::uint32_t>)) {
            eprintf_warn("No color plate data to recover from - tag likely extracted");
            std::exit(EXIT_FAILURE);
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
        
        auto fail = []() {
            eprintf_error("Color plate data could not be decompressed");
            std::exit(EXIT_FAILURE);
        };
        
        // Zstandard if extended
        if(invader_bitmap) {
            if(ZSTD_decompress(pixels.data(), image_size, compressed_data, compressed_size) != image_size) {
                fail();
            }
        }

        // DEFLATE if not extended
        else {
            z_stream inflate_stream;
            inflate_stream.zalloc = Z_NULL;
            inflate_stream.zfree = Z_NULL;
            inflate_stream.opaque = Z_NULL;
            inflate_stream.avail_out = image_size;
            inflate_stream.next_out = reinterpret_cast<Bytef *>(pixels.data());
            inflate_stream.avail_in = compressed_size;
            inflate_stream.next_in = reinterpret_cast<Bytef *>(const_cast<std::byte *>(compressed_data));

            // Do it
            inflateInit(&inflate_stream);
            inflate(&inflate_stream, Z_FINISH);
            inflateEnd(&inflate_stream);
        }
        
        auto file_path = data / (path + ".tif");
        auto file_path_str = file_path.string();
        create_directories_for_path(file_path);
        
        // Let's begin
        auto *tiff = TIFFOpen(file_path_str.c_str(), "w");
        if(!tiff) {
            eprintf_error("Failed to open %s for writing", file_path_str.c_str());
            std::exit(EXIT_FAILURE);
        }
        
        TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, width); 
        TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, height); 
        TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 8, 8, 8, 8); 
        TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 4); 
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
    
    void recover(const Parser::ParserStruct &tag, const std::string &path, const std::filesystem::path &data, HEK::TagFourCC tag_fourcc) {
        recover_tag_collection(tag, path, data);
        recover_bitmap(tag, path, data);
    
        eprintf_warn("Data cannot be recovered from tag class %s", HEK::tag_fourcc_to_extension(tag_fourcc));
        std::exit(EXIT_FAILURE);
    }
}
